#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/PipelineStateObject_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Core/Logger.h"

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Graphics/RenderTarget.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			PipelineLayoutManager_Vulkan::PipelineLayoutManager_Vulkan()
			{ }

			PipelineLayoutManager_Vulkan::~PipelineLayoutManager_Vulkan()
			{ }

			vk::PipelineLayout PipelineLayoutManager_Vulkan::GetOrCreateLayout(PipelineStateObject pso)
			{
				assert(m_context != nullptr);

				const std::vector<Descriptor> descriptors = pso.Shader->GetDescriptors();
				u64 hash = 0;
				for (const Descriptor& descriptor : descriptors)
				{
					HashCombine(hash, descriptor.GetHash(false));
				}

				auto itr = m_layouts.find(hash);
				if (itr != m_layouts.end())
				{
					return itr->second;
				}

				std::vector<vk::DescriptorSetLayout> setLayouts = {};

				int currentSet = 0;
				std::vector<Descriptor> currentDescriptors;

				for (const Descriptor& descriptor : descriptors)
				{
					if (descriptor.Type == DescriptorType::Sampler)
					{
						continue;
					}

					if (currentSet != descriptor.Set)
					{
						RHI_DescriptorLayout_Vulkan* layoutVulkan = dynamic_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(currentSet, currentDescriptors));
						currentDescriptors.clear();
						currentSet = descriptor.Set;

						setLayouts.push_back(layoutVulkan->GetLayout());
					}
					currentDescriptors.push_back(descriptor);
				}
				RHI_DescriptorLayout_Vulkan* layoutVulkan = dynamic_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(currentSet, currentDescriptors));
				setLayouts.push_back(layoutVulkan->GetLayout());


				vk::PipelineLayoutCreateInfo createInfo = vk::PipelineLayoutCreateInfo(
					{},
					setLayouts,
					{});
				vk::PipelineLayout layout = m_context->GetDevice().createPipelineLayout(createInfo);
				m_layouts[hash] = layout;

				return layout;
			}

			void PipelineLayoutManager_Vulkan::Destroy()
			{
				for (const auto pair : m_layouts)
				{
					m_context->GetDevice().destroyPipelineLayout(pair.second);
				}
				m_layouts.clear();
			}

			PipelineStateObjectManager_Vulkan::PipelineStateObjectManager_Vulkan()
			{ }

			PipelineStateObjectManager_Vulkan::~PipelineStateObjectManager_Vulkan()
			{ }

			vk::Pipeline PipelineStateObjectManager_Vulkan::GetOrCreatePSO(PipelineStateObject pso)
			{
				assert(m_context != nullptr);

				const u64 psoHash = pso.GetHash();
				auto itr = m_pipelineStateObjects.find(psoHash);
				if (itr != m_pipelineStateObjects.end())
				{
					return itr->second;
				}

				// Create pipeline layout.
				// Create descriptor set layout
				vk::PipelineLayout layout = m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso);

				RHI_Shader_Vulkan* shaderVulkan = dynamic_cast<RHI_Shader_Vulkan*>(pso.Shader);
				std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
				std::array<std::string, ShaderStageCount> shaderFuncNames;
				for (int i = 0; i < ShaderStageCount; ++i)
				{
					ShaderStageFlagBits shaderStage =static_cast<ShaderStageFlagBits>(1 << i);
					vk::ShaderModule shaderModule = shaderVulkan->GetStage(shaderStage);
					if (!shaderModule)
					{
						continue;
					}
			
					for (const wchar_t wChar : shaderVulkan->GetMainFuncName(shaderStage))
					{
						shaderFuncNames[i].push_back((char)wChar);
					}

					vk::PipelineShaderStageCreateInfo info({}, ShaderStageFlagBitsToVulkan(shaderStage),
						shaderModule, 
						shaderFuncNames[i].c_str());
					pipelineShaderStageCreateInfos.push_back(info);
				}
				const VertexInputLayout_Vulkan& pipelineVertexInputStateCreateInfo = shaderVulkan->GetVertexInputLayout();

				vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(),
					PrimitiveTopologyTypeToVulkan(pso.PrimitiveTopologyType));
			
				vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);
			
				vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
					vk::PipelineRasterizationStateCreateFlags(),					// flags
					pso.DepthClampEnabled,											// depthClampEnable
					false,															// rasterizerDiscardEnable
					vk::PolygonMode::eFill,											// polygonMode
					CullModeToVulkan(pso.CullMode),									// cullMode
					vk::FrontFace::eClockwise,										// frontFace
					pso.DepthBaisEnabled,											// depthBiasEnable
					0.0f,															// depthBiasConstantFactor
					0.0f,															// depthBiasClamp
					0.0f,															// depthBiasSlopeFactor
					1.0f															// lineWidth
				);
			
				vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
					vk::PipelineMultisampleStateCreateFlags(),  // flags
					vk::SampleCountFlagBits::e1                 // rasterizationSamples
																// other values can be default
				);
			
				vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
				vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
					vk::PipelineDepthStencilStateCreateFlags(),  // flags
					pso.DepthTest,								 // depthTestEnable
					pso.DepthWrite,								 // depthWriteEnable
					vk::CompareOp::eLessOrEqual,                 // depthCompareOp
					false,                                       // depthBoundTestEnable
					false,                                       // stencilTestEnable
					stencilOpState,                              // front
					stencilOpState                               // back
				);
			
				vk::ColorComponentFlags colorComponentFlags = ColourComponentFlagsToVulkan(pso.ColourWriteMask);
				vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
					pso.BlendEnable ,									// blendEnable
					BlendFactorToVulkan(pso.SrcColourBlendFactor),		// srcColorBlendFactor
					BlendFactorToVulkan(pso.DstColourBlendFactor),		// dstColorBlendFactor
					BlendOpToVulkan(pso.ColourBlendOp),					// colorBlendOp
					BlendFactorToVulkan(pso.SrcAplhaBlendFactor),		// srcAlphaBlendFactor
					BlendFactorToVulkan(pso.DstAplhaBlendFactor),		// dstAlphaBlendFactor
					BlendOpToVulkan(pso.AplhaBlendOp),					// alphaBlendOp
					colorComponentFlags									// colorWriteMask
				);
			
				vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
					vk::PipelineColorBlendStateCreateFlags(),  // flags
					false,                                     // logicOpEnable
					vk::LogicOp::eNoOp,                        // logicOp
					pipelineColorBlendAttachmentState,         // attachments
					{ { 1.0f, 1.0f, 1.0f, 1.0f } }             // blendConstants
				);
			
				std::array<vk::DynamicState, 2> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
				vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), dynamicStates);
			
				vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
					vk::PipelineCreateFlags(),																	// flags
					pipelineShaderStageCreateInfos,																// stages
					&pipelineVertexInputStateCreateInfo.CreateInfo,												// pVertexInputState
					&pipelineInputAssemblyStateCreateInfo,														// pInputAssemblyState
					nullptr,																					// pTessellationState
					&pipelineViewportStateCreateInfo,															// pViewportState
					&pipelineRasterizationStateCreateInfo,														// pRasterizationState
					&pipelineMultisampleStateCreateInfo,														// pMultisampleState
					&pipelineDepthStencilStateCreateInfo,														// pDepthStencilState
					&pipelineColorBlendStateCreateInfo,															// pColorBlendState
					&pipelineDynamicStateCreateInfo,															// pDynamicState
					layout,																						// layout
					m_context->GetRenderpassManager().GetOrCreateRenderpass({ pso.RenderTargets, pso.DepthStencil })				// renderPass
				);

				vk::ResultValue<vk::Pipeline> result = m_context->GetDevice().createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
				m_pipelineStateObjects[psoHash] = result.value;

				return result.value;
			}

			void PipelineStateObjectManager_Vulkan::Destroy()
			{
				for (const auto pair : m_pipelineStateObjects)
				{
					m_context->GetDevice().destroyPipeline(pair.second);
				}
				m_pipelineStateObjects.clear();
			}


			RenderpassManager_Vulkan::~RenderpassManager_Vulkan()
			{
				if (m_renderpasses.size() > 0)
				{
					IS_CORE_ERROR("[GPURenderpassManager_Vulkan::~GPURenderpassManager_Vulkan] Not all renderpass have not been destroyed.");
				}
			}

			vk::RenderPass RenderpassManager_Vulkan::GetOrCreateRenderpass(RenderpassDesc_Vulkan desc)
			{
				assert(m_context != nullptr);

				const u64 hash = desc.GetHash();
				auto itr = m_renderpasses.find(hash);
				if (itr != m_renderpasses.end())
				{
					return itr->second;
				}

				std::array<vk::AttachmentDescription, 8> attachmentDescriptions;
				std::array<vk::AttachmentReference, 8> colorAttachments;
				vk::AttachmentReference depthAttachment;
				std::vector<vk::SubpassDependency> subpassDependenices;

				int attachmentIndex = 0;
				int colourIndex = 0;
				for (RenderTarget* rt : desc.RenderTargets)
				{
					if (rt)
					{
						attachmentDescriptions[attachmentIndex] = vk::AttachmentDescription(
							vk::AttachmentDescriptionFlags(),
							PixelFormatToVulkan(rt->GetTexture()->GetFormat()),
							vk::SampleCountFlagBits::e1,
							vk::AttachmentLoadOp::eClear,
							vk::AttachmentStoreOp::eStore,
							vk::AttachmentLoadOp::eDontCare,
							vk::AttachmentStoreOp::eDontCare,
							vk::ImageLayout::eUndefined,
							vk::ImageLayout::ePresentSrcKHR);
						colorAttachments[colourIndex] = vk::AttachmentReference(vk::AttachmentReference(attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal));
						++attachmentIndex;
						++colourIndex;
					}
				}

				// No render targets, guess we are swapchain.
				if (attachmentIndex == 0)
				{
					// Assume this is a swapchain render pass as no user defined render targets are added.
					attachmentDescriptions[attachmentIndex] = vk::AttachmentDescription(
						vk::AttachmentDescriptionFlags(),
						m_context->GetSwapchainColourFormat(),
						vk::SampleCountFlagBits::e1,
						vk::AttachmentLoadOp::eClear,
						vk::AttachmentStoreOp::eStore,
						vk::AttachmentLoadOp::eDontCare,
						vk::AttachmentStoreOp::eDontCare,
						vk::ImageLayout::eUndefined,
						vk::ImageLayout::ePresentSrcKHR);
					colorAttachments[colourIndex] = vk::AttachmentReference(vk::AttachmentReference(attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal));
					++attachmentIndex;
					++colourIndex;
				}

				subpassDependenices.push_back(
					vk::SubpassDependency(
						VK_SUBPASS_EXTERNAL,
						0u,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::PipelineStageFlagBits::eColorAttachmentOutput,
						vk::AccessFlagBits::eNoneKHR,
						vk::AccessFlagBits::eColorAttachmentWrite,
						{ }
					));

				if (desc.DepthStencil)
				{
					attachmentDescriptions[attachmentIndex] = vk::AttachmentDescription(
						vk::AttachmentDescriptionFlags(),
						PixelFormatToVulkan(desc.DepthStencil->GetTexture()->GetFormat()),
						vk::SampleCountFlagBits::e1,								// Sample count
						vk::AttachmentLoadOp::eClear,								// load op
						vk::AttachmentStoreOp::eStore,								// store op
						vk::AttachmentLoadOp::eDontCare,							// stencil load op
						vk::AttachmentStoreOp::eDontCare,							// stencil store op
						vk::ImageLayout::eUndefined,								// initial layout
						vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal);	// final layout

					depthAttachment.attachment = static_cast<int>(attachmentIndex);
					depthAttachment.layout = vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
					++attachmentIndex;

					subpassDependenices.push_back(
						vk::SubpassDependency(
							VK_SUBPASS_EXTERNAL,
							0u,
							vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
							vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
							vk::AccessFlagBits::eNoneKHR,
							vk::AccessFlagBits::eDepthStencilAttachmentWrite,
							{ }
						));
				}
					 
				vk::SubpassDescription  subpassDescription(
					vk::SubpassDescriptionFlags(),
					vk::PipelineBindPoint::eGraphics,
					0, nullptr,
					colourIndex, colorAttachments.data(),
					nullptr,
					desc.DepthStencil ? &depthAttachment : nullptr,
					0, nullptr);

				vk::RenderPass renderPass = m_context->GetDevice().createRenderPass(
					vk::RenderPassCreateInfo(
						vk::RenderPassCreateFlags(),
						attachmentIndex, attachmentDescriptions.data(),
						1, &subpassDescription, 
						1, subpassDependenices.data()));
				m_renderpasses[hash] = renderPass;
				return renderPass;
			}

			void RenderpassManager_Vulkan::Destroy()
			{
				for (const auto& pair : m_renderpasses)
				{
					m_context->GetDevice().destroyRenderPass(pair.second);
				}
				m_renderpasses.clear();
			}
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)