#include "Graphics/RHI/RHI_Renderpass.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/RHI_Texture.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include <vulkan/vulkan.hpp>
#endif /// #ifdef IS_VULKAN_ENABLED
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#endif /// #ifdef IS_DX12_ENABLED

#include "Core/Profiler.h"

#include <tuple>

namespace Insight
{
	namespace Graphics
	{
		u64 AttachmentDescription::GetHash() const
		{
			u64 hash = 0;

			HashCombine(hash, Format);
			HashCombine(hash, LoadOp);
			HashCombine(hash, StoreOp);
			HashCombine(hash, StencilLoadOp);
			HashCombine(hash, StencilStoreOp);
			HashCombine(hash, InitalLayout);
			HashCombine(hash, FinalLayout);

			return hash;
		}

		u64 RenderpassDescription::GetHash() const
		{
			u64 hash = 0;
			for (const auto& texture : ColourAttachments)
			{
				HashCombine(hash, texture->GetInfo().Format);
			}
			for (const auto& attachment : Attachments)
			{
				HashCombine(hash, attachment.GetHash());
			}

			if (DepthStencil)
			{
				HashCombine(hash, DepthStencil);
				HashCombine(hash, DepthStencilAttachment.GetHash());
			}

			HashCombine(hash, AllowDynamicRendering);
			HashCombine(hash, SwapchainPass);

			return hash;
		}

		void RHI_RenderpassManager::SetRenderContext(RenderContext* context)
		{
			m_context = context;
		}

		RHI_Renderpass RHI_RenderpassManager::GetOrCreateRenderpass(RenderpassDescription& description)
		{
			PrepreRenderpass(description);
			if (auto itr = m_renderpasses.find(description.GetHash()); itr != m_renderpasses.end())
			{
				return itr->second;
			}
			return AddRenderpass(description);
		}

		RHI_Renderpass RHI_RenderpassManager::GetRenderpass(u64 hash) const
		{
			if (auto itr = m_renderpasses.find(hash); itr != m_renderpasses.end())
			{
				return itr->second;
			}
			return { };
		}


		void RHI_RenderpassManager::Release(u64 hash, bool remove)
		{
			if (auto itr = m_renderpasses.find(hash); itr != m_renderpasses.end())
			{
#ifdef IS_VULKAN_ENABLED
				if (GraphicsManager::IsVulkan())
				{
					RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);
					VkRenderPass renderpassVulkan = reinterpret_cast<VkRenderPass>(itr->second.Resource);
					vkDestroyRenderPass(contextVulkan->GetDevice(), renderpassVulkan, nullptr);
					itr->second.Resource = nullptr;
				}
#endif

				if (remove)
				{
					m_renderpasses.erase(itr);
				}
			}
		}

		void RHI_RenderpassManager::Release(RenderpassDescription description, bool remove)
		{
			Release(description.GetHash(), remove);
		}

		void RHI_RenderpassManager::ReleaseAll()
		{
			for (const auto& pair : m_renderpasses)
			{
				Release(pair.first, false);
			}
			m_renderpasses.clear();
		}

		void RHI_RenderpassManager::PrepreRenderpass(RenderpassDescription& description)
		{
			u32 colourImageIndex = 0;
			for (const auto& colourImage : description.ColourAttachments)
			{
				/// Find if an image is missing an attachment. If so then add a default 
				/// attachment. This needs to be done as the attachments are used later in rendering.
				if (description.Attachments.size() <= colourImageIndex)
				{
					description.Attachments.push_back(AttachmentDescription::Default(colourImage->GetFormat(), ImageLayout::ColourAttachment));
					description.Attachments.back().ClearColour = description.Pso->RenderTargetClearValues[colourImageIndex];
				}
				++colourImageIndex;
			}

			if (description.DepthStencil && !description.DepthStencilAttachment.IsValid())
			{
				description.DepthStencilAttachment = AttachmentDescription::Default(description.DepthStencil->GetFormat(), ImageLayout::DepthStencilAttachment);
				description.DepthStencilAttachment.DepthStencilClear = description.Pso->DepthSteniclClearValue;
			}

			if (description.SwapchainPass)
			{
				if (description.ColourAttachments.size() == 0)
				{
					description.ColourAttachments.push_back(m_context->GetSwaphchainIamge());
				}
				if (description.Attachments.size() == 0)
				{
					description.Attachments.push_back(AttachmentDescription::Default(m_context->GetSwaphchainIamge()->GetFormat(), ImageLayout::ColourAttachment));
				}
			}
		}

		RHI_Renderpass RHI_RenderpassManager::AddRenderpass(RenderpassDescription& description)
		{
			IS_PROFILE_FUNCTION();
#ifdef IS_VULKAN_ENABLED
			if (GraphicsManager::IsVulkan())
			{
				RHI::Vulkan::RenderContext_Vulkan* contextVulkan = static_cast<RHI::Vulkan::RenderContext_Vulkan*>(m_context);

				struct AttachmentCreation
				{
					static VkAttachmentDescription CreateCustom(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						VkAttachmentDescription description = {};
						description.format = texture != nullptr ?
							PixelFormatToVulkan(texture->GetFormat())
							: PixelFormatToVulkan(attachment->Format);
						description.samples = VK_SAMPLE_COUNT_1_BIT;
						description.loadOp = AttachmentLoadOpToVulkan(attachment->LoadOp);
						description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						description.stencilLoadOp = AttachmentLoadOpToVulkan(attachment->StencilLoadOp);
						description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
						description.initialLayout = ImageLayoutToVulkan(attachment->InitalLayout);
						description.finalLayout = ImageLayoutToVulkan(attachment->FinalLayout);
						return description;
					}

					static VkAttachmentDescription CreateColour(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						if (attachment == nullptr)
						{
							VkAttachmentDescription description = {};
							description.format = texture != nullptr ?
								PixelFormatToVulkan(texture->GetFormat())
								: PixelFormatToVulkan(attachment->Format);
							description.samples = VK_SAMPLE_COUNT_1_BIT;
							description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
							description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
							description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
							description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
							description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							return description;
						}
						VkAttachmentDescription desc = CreateCustom(texture, attachment);
						return desc;
					}

					static VkAttachmentDescription CreateDepthStencil(const RHI_Texture* texture, const AttachmentDescription* attachment)
					{
						if (attachment == nullptr)
						{
							VkAttachmentDescription description = {};
							description.format = texture != nullptr ?
								PixelFormatToVulkan(texture->GetFormat())
								: PixelFormatToVulkan(attachment->Format);
							description.samples = VK_SAMPLE_COUNT_1_BIT;
							description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
							description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
							description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
							description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
							description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
							description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
							return description;
						}
						VkAttachmentDescription desc = CreateCustom(texture, attachment);
						desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
						return desc;
					}

					static void AddRenderpassDescriptionAttachmentData(std::vector<AttachmentDescription>& descriptions, VkAttachmentDescription descriptionVulkan)
					{
						AttachmentDescription newDescription = { };
						newDescription.Format = PixelFormat::D32_Float;/// VkFormatToPixelFormat[(int)descriptionVulkan.format];
						newDescription.LoadOp = VkToAttachmentLoadOp(descriptionVulkan.loadOp);
						newDescription.StoreOp = AttachmentStoreOp::Store;
						newDescription.StencilLoadOp = VkToAttachmentLoadOp(descriptionVulkan.stencilLoadOp);
						newDescription.StencilStoreOp = AttachmentStoreOp::DontCare;
						newDescription.InitalLayout = VkToImageLayout(descriptionVulkan.initialLayout);
						newDescription.FinalLayout = VkToImageLayout(descriptionVulkan.finalLayout);
						descriptions.push_back(newDescription);
					}

					static std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>>
						CreateAllColour(std::vector<RHI_Texture*>& textures, std::vector<AttachmentDescription>& descriptions)
					{
						std::vector<VkAttachmentDescription> resultAttachments;
						std::vector<VkAttachmentReference> resultReferences;

						for (size_t i = 0; i < textures.size(); ++i)
						{
							const AttachmentDescription* attachment = nullptr;
							if (i >= 0 && i < descriptions.size())
							{
								attachment = &descriptions.at(i);
							}
							resultAttachments.push_back(CreateColour(textures.at(i), attachment));
							resultReferences.push_back(VkAttachmentReference{ static_cast<u32>(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
							if (!attachment)
							{
								AddRenderpassDescriptionAttachmentData(descriptions, resultAttachments.at(resultAttachments.size() - 1ull));
							}
						}

						if (textures.size() == 0 && descriptions.size() > 0)
						{
							resultAttachments.push_back(CreateColour(nullptr, &descriptions.at(0)));
							resultAttachments.at(0).format = resultAttachments.at(0).format != VkFormat::VK_FORMAT_UNDEFINED ?
								resultAttachments.at(0).format
								: static_cast<RHI::Vulkan::RenderContext_Vulkan*>(GraphicsManager::Instance().GetRenderContext())->GetSwapchainColourFormat();
							resultReferences.push_back(VkAttachmentReference{ static_cast<u32>(0), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
						}

						return { resultAttachments, resultReferences };
					}

					static VkAttachmentReference CreateDepthStencil(std::vector<VkAttachmentDescription>& attachmentDescriptions
						, RHI_Texture* texture
						, AttachmentDescription& attachmentDescription)
					{
						VkAttachmentReference result = { };

						if (texture)
						{
							result.attachment = static_cast<u32>(attachmentDescriptions.size());
							result.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
							attachmentDescriptions.push_back(CreateDepthStencil(texture, attachmentDescription.IsValid() ?
								&attachmentDescription : nullptr));

							if (!attachmentDescription.IsValid())
							{
								std::vector<AttachmentDescription> descriptions;
								AddRenderpassDescriptionAttachmentData(descriptions
									, attachmentDescriptions.at(attachmentDescriptions.size() - 1ull));
								attachmentDescription = descriptions.at(0);
							}
						}

						return result;
					}
				};

				struct SubpassCreation
				{
					static std::vector<VkSubpassDependency> CreateDependencies(bool hasDepthStencil)
					{
						std::vector<VkSubpassDependency> result;

						result.push_back(
							VkSubpassDependency
							{
								VK_SUBPASS_EXTERNAL,
								0u,
								VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
								VK_ACCESS_NONE,
								VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
								{ }
							});

						if (hasDepthStencil)
						{
							result.push_back(
								VkSubpassDependency
								{
									VK_SUBPASS_EXTERNAL,
									0u,
									VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
									VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
									VK_ACCESS_NONE,
									VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
									{ }
								});
						}

						return result;
					}
				};

				std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> colourAttachmentsAndRefs
					= AttachmentCreation::CreateAllColour(description.ColourAttachments, description.Attachments);
				std::vector<VkAttachmentDescription>& renderpassAttachments = colourAttachmentsAndRefs.first;
				std::vector<VkAttachmentReference>& colourReferences = colourAttachmentsAndRefs.second;

				if (description.SwapchainPass)
				{
					/// If there are no render attachments added and we have a custom attachment add custom attachment.
					if (renderpassAttachments.size() == 0 && description.Attachments.size() > 0)
					{
						renderpassAttachments.push_back(AttachmentCreation::CreateCustom(nullptr, &description.Attachments.at(0)));

						colourReferences.push_back(VkAttachmentReference{ static_cast<u32>(renderpassAttachments.size() - 1), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

						description.ColourAttachments.push_back(contextVulkan->GetSwaphchainIamge());
					}
					else if (renderpassAttachments.size() == 0)
					{
						renderpassAttachments.push_back(AttachmentCreation::CreateColour(nullptr,
							&AttachmentDescription::Default(VkFormatToPixelFormat[(int)contextVulkan->GetSwapchainColourFormat()], ImageLayout::PresentSrc)));

						colourReferences.push_back(VkAttachmentReference{ static_cast<u32>(renderpassAttachments.size() - 1), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
						AttachmentCreation::AddRenderpassDescriptionAttachmentData(description.Attachments, renderpassAttachments.at(renderpassAttachments.size() - 1ull));

						description.ColourAttachments.push_back(contextVulkan->GetSwaphchainIamge());
					}
				}

				VkAttachmentReference depthReference = AttachmentCreation::CreateDepthStencil(renderpassAttachments
					, description.DepthStencil
					, description.DepthStencilAttachment);

				std::vector<VkSubpassDependency> subpassDependencies = SubpassCreation::CreateDependencies(description.DepthStencil != nullptr);

				VkSubpassDescription  subpassDescription = { };
				subpassDescription.colorAttachmentCount = static_cast<u32>(colourReferences.size());
				subpassDescription.pColorAttachments = colourReferences.data();
				if (description.DepthStencil)
				{
					subpassDescription.pDepthStencilAttachment = &depthReference;
				}

				VkRenderPassCreateInfo createInfo = { };
				createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				createInfo.attachmentCount = static_cast<u32>(renderpassAttachments.size());
				createInfo.pAttachments = renderpassAttachments.data();
				createInfo.subpassCount = 1ul;
				createInfo.pSubpasses = &subpassDescription;
				createInfo.dependencyCount = static_cast<u32>(subpassDependencies.size());
				createInfo.pDependencies = subpassDependencies.data();

				VkRenderPass renderpass;
				ThrowIfFailed(vkCreateRenderPass(contextVulkan->GetDevice(), &createInfo, nullptr, &renderpass));
				RHI_Renderpass newPass;
				newPass.Resource = renderpass;

				const u64 hash = description.GetHash();
				ASSERT(m_renderpasses.find(hash) == m_renderpasses.end());
				m_renderpasses[hash] = newPass;
			}
#endif /// #ifdef IS_VULKAN_ENABLED

			return m_renderpasses[description.GetHash()];
		}
	}
}