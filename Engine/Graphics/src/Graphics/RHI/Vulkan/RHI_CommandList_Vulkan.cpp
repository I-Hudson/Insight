#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Pipeline_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_PipelineLayout_Vulkan.h"
#include "Graphics/Window.h"

#include "Graphics/RenderTarget.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			//// <summary>
			/// RHI_CommandList_Vulkan
			//// </summary>
			void RHI_CommandList_Vulkan::Create(RenderContext* context)
			{
				m_context = context;
				m_context_vulkan = static_cast<RenderContext_Vulkan*>(context);

				m_cmdBeginDebugUtilsLabelEXT = static_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(m_context_vulkan->GetExtensionFunction("vkCmdBeginDebugUtilsLabelEXT"));
				m_cmdEndDebugUtilsLabelEXT = static_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(m_context_vulkan->GetExtensionFunction("vkCmdEndDebugUtilsLabelEXT"));
			}

			void RHI_CommandList_Vulkan::PipelineBarrier(Graphics::PipelineBarrier barrier)
			{
				std::vector<VkBufferMemoryBarrier> bufferBarriers;
				for (const BufferBarrier& bufferBarrier : barrier.BufferBarriers)
				{
					VkBufferMemoryBarrier b = { };
					b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
					b.srcAccessMask = AccessFlagsToVulkan(bufferBarrier.SrcAccessFlags);
					b.dstAccessMask = AccessFlagsToVulkan(bufferBarrier.DstAccessFlags);
					b.buffer = static_cast<RHI_Buffer_Vulkan*>(bufferBarrier.Buffer)->GetBuffer();
					b.offset = bufferBarrier.Offset;
					b.size = bufferBarrier.Size;

					bufferBarriers.push_back(std::move(b));
				}

				std::vector<VkImageMemoryBarrier> imageBarriers;
				for (const ImageBarrier& imageBarrier : barrier.ImageBarriers)
				{
					VkImageMemoryBarrier b = { };
					b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					b.srcAccessMask = AccessFlagsToVulkan(imageBarrier.SrcAccessFlags);
					b.dstAccessMask = AccessFlagsToVulkan(imageBarrier.DstAccessFlags);
					b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					b.oldLayout = ImageLayoutToVulkan(imageBarrier.OldLayout);
					b.newLayout = ImageLayoutToVulkan(imageBarrier.NewLayout);
					b.image = static_cast<RHI_Texture_Vulkan*>(imageBarrier.Image)->GetImage();

					b.subresourceRange.aspectMask = ImageAspectFlagsToVulkan(imageBarrier.SubresourceRange.AspectMask);
					b.subresourceRange.baseMipLevel = imageBarrier.SubresourceRange.BaseMipLevel;
					b.subresourceRange.levelCount = imageBarrier.SubresourceRange.LevelCount;
					b.subresourceRange.baseArrayLayer = imageBarrier.SubresourceRange.BaseArrayLayer;
					b.subresourceRange.layerCount = imageBarrier.SubresourceRange.LayerCount;

					imageBarrier.Image->SetLayout(imageBarrier.NewLayout);

					imageBarriers.push_back(std::move(b));
				}
				PipelineBarrier(barrier.SrcStage
					, barrier.DstStage
					, bufferBarriers
				, imageBarriers);
			}

			void RHI_CommandList_Vulkan::PipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage
				, std::vector<VkBufferMemoryBarrier> const& bufferMemoryBarrier, std::vector<VkImageMemoryBarrier> const& imageMemoryBarrier)
			{
				vkCmdPipelineBarrier(
					m_commandList,
					PipelineStageFlagsToVulkan(srcStage),
					PipelineStageFlagsToVulkan(dstStage),
					VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT,
					0, nullptr,
					static_cast<u32>(bufferMemoryBarrier.size()), bufferMemoryBarrier.data(),
					static_cast<u32>(imageMemoryBarrier.size()), imageMemoryBarrier.data());
				RenderStats::Instance().PipelineBarriers++;
			}

			void RHI_CommandList_Vulkan::PipelineBarrierBuffer(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<VkBufferMemoryBarrier> const& bufferMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, bufferMemoryBarrier, { });
			}

			void RHI_CommandList_Vulkan::PipelineBarrierImage(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<VkImageMemoryBarrier> const& imageMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, { }, imageMemoryBarrier);
			}

			void RHI_CommandList_Vulkan::Reset()
			{
				RHI_CommandList::Reset();
				for (auto& pair : m_framebuffers)
				{
					vkDestroyFramebuffer(m_context_vulkan->GetDevice(), pair.second, nullptr);
				}
				m_framebuffers.clear();
				m_boundDescriptors = 0;
			}

			void RHI_CommandList_Vulkan::Close()
			{
				if (m_activeRenderpass)
				{
					EndRenderpass();
				}

				if (m_commandList)
				{
					vkEndCommandBuffer(m_commandList);
					m_state = RHI_CommandListStates::Ended;
				}
			}

			void RHI_CommandList_Vulkan::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Buffer_Vulkan* dstVulkan = static_cast<RHI_Buffer_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = static_cast<RHI_Buffer_Vulkan*>(src);
				std::array<VkBufferCopy, 1> copyRegion =
				{
					VkBufferCopy { 0, offset, src->GetSize() }
				};
				vkCmdCopyBuffer(m_commandList, srcVulkan->GetBuffer(), dstVulkan->GetBuffer(), static_cast<u32>(copyRegion.size()), copyRegion.data());
			}

			void RHI_CommandList_Vulkan::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Texture_Vulkan* dstVulkan = static_cast<RHI_Texture_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = static_cast<RHI_Buffer_Vulkan*>(src);
				std::array<VkBufferImageCopy, 1> copyRegion =
				{
					VkBufferImageCopy{
						offset,
						0,
						0,
						VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
						VkOffset3D{ 0, 0, 0 },
						VkExtent3D{ static_cast<u32>(dst->GetWidth()), static_cast<u32>(dst->GetHeight()), 1 } }
				};
				VkImageMemoryBarrier memoryBarriers = {};
				memoryBarriers.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				memoryBarriers.srcAccessMask = VK_ACCESS_NONE;
				memoryBarriers.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				memoryBarriers.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				memoryBarriers.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				memoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				memoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				memoryBarriers.image = dstVulkan->GetImage();
				memoryBarriers.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

				PipelineBarrierImage(+PipelineStageFlagBits::TopOfPipe, +PipelineStageFlagBits::Transfer, { memoryBarriers });
				vkCmdCopyBufferToImage(m_commandList, srcVulkan->GetBuffer(), dstVulkan->GetImage(), memoryBarriers.newLayout, static_cast<u32>(copyRegion.size()), copyRegion.data());

				memoryBarriers.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				memoryBarriers.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				memoryBarriers.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				memoryBarriers.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				PipelineBarrierImage(+PipelineStageFlagBits::Transfer, +PipelineStageFlagBits::FragmentShader, { memoryBarriers });
			}

			void RHI_CommandList_Vulkan::Release()
			{
				if (m_commandList)
				{
					for (auto& pair : m_framebuffers)
					{
						vkDestroyFramebuffer(m_context_vulkan->GetDevice(), pair.second, nullptr);
					}
					m_framebuffers.clear();

					vkFreeCommandBuffers(m_context_vulkan->GetDevice(), m_allocator->GetAllocator(), 1, &m_commandList);
					m_commandList = nullptr;
				}
			}

			bool RHI_CommandList_Vulkan::ValidResource()
			{
				return m_commandList;
			}

			void RHI_CommandList_Vulkan::SetName(std::string name)
			{
				m_context_vulkan->SetObjectName(name, (u64)m_commandList, VK_OBJECT_TYPE_COMMAND_BUFFER);
			}

			void RHI_CommandList_Vulkan::BeginRenderpass(RenderpassDescription renderDescription)
			{
				IS_PROFILE_FUNCTION();

				if (!m_activeRenderpass)
				{
					////// Before rendering anything, make sure all our RenderTargets are in the correct layout.
					///for (size_t i = 0; i < pso.RenderTargets.size(); ++i)
					///{
					///	const RenderTarget* rt = pso.RenderTargets.at(i);
					///	if (rt)
					///	{
					///		const RHI_Texture_Vulkan* textureVulkan = static_cast<const RHI_Texture_Vulkan*>(rt->GetTexture());

					///		VkImageMemoryBarrier barrier = VkImageMemoryBarrier(
					///			VkAccessFlagBits::eNoneKHR,
					///			VkAccessFlagBits::eColorAttachmentWrite,
					///			VkImageLayout::eUndefined,
					///			VkImageLayout::eColorAttachmentOptimal,
					///			RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					///			RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					///			textureVulkan->GetImage(),
					///			VkImageSubresourceRange(VkImageAspectFlagBits::eColor, 0, 1, 0, 1));
					///		PipelineBarrierImage(PipelineStageFlagBits::TopOfPipe, PipelineStageFlagBits::ColourAttachmentOutput, { barrier });
					///	}
					///}

					VkRect2D rect = VkRect2D{ { }, { (u32)m_drawData.Viewport.x, (u32)m_drawData.Viewport.y } };

					if (renderDescription.AllowDynamicRendering 
						&& m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
					{
						m_dynamicRendering = true;

						std::vector<VkRenderingAttachmentInfo> colourAttachments;
						VkRenderingAttachmentInfo depthAttachment = {};
						depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
						VkRenderingAttachmentInfo detencilAttacment = {};
						detencilAttacment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;

						auto makeRenderingAttachment =
							[](const RHI_Texture* texture, const AttachmentDescription& attachment_description) -> VkRenderingAttachmentInfo
						{
							std::array<float, 4> clearColourValues = {
								attachment_description.ClearColour.x
								, attachment_description.ClearColour.y
								, attachment_description.ClearColour.z
								, attachment_description.ClearColour.w };


							VkRenderingAttachmentInfo attachment = { };
							attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
							attachment.imageView = static_cast<const RHI_Texture_Vulkan*>(texture)->GetImageView(attachment_description.Layer_Array_Index);
							attachment.imageLayout = ImageLayoutToVulkan(attachment_description.FinalLayout);
							attachment.loadOp = AttachmentLoadOpToVulkan(attachment_description.LoadOp);
							attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
							attachment.clearValue.color.float32[0] = clearColourValues[0];
							attachment.clearValue.color.float32[1] = clearColourValues[1];
							attachment.clearValue.color.float32[2] = clearColourValues[2];
							attachment.clearValue.color.float32[3] = clearColourValues[3];

							attachment.imageLayout = attachment.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ?
								VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachment.imageLayout;

							return attachment;
						};

						VkRenderingInfo renderingInfo = { };
						renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
						renderingInfo.renderArea = rect;
						renderingInfo.layerCount = 1;

						int descriptionIndex = 0;
						for (RHI_Texture* texture : renderDescription.ColourAttachments)
						{
							IS_PROFILE_SCOPE("Colour attachments");
							VkRenderingAttachmentInfo attachment = makeRenderingAttachment(texture, renderDescription.Attachments.at(descriptionIndex));
							colourAttachments.push_back(attachment);
							texture->SetLayout(renderDescription.Attachments.at(descriptionIndex).FinalLayout);
							++descriptionIndex;
						}
						renderingInfo.pColorAttachments = colourAttachments.data();
						renderingInfo.colorAttachmentCount = static_cast<u32>(colourAttachments.size());

						if (renderDescription.DepthStencil)
						{
							IS_PROFILE_SCOPE("Depth Stencil attachments");

							depthAttachment = makeRenderingAttachment(renderDescription.DepthStencil, renderDescription.DepthStencilAttachment);
							depthAttachment.clearValue = { };
							depthAttachment.clearValue.depthStencil = VkClearDepthStencilValue{
								renderDescription.DepthStencilAttachment.DepthStencilClear.x,
								static_cast<u32>(renderDescription.DepthStencilAttachment.DepthStencilClear.y) };
							renderingInfo.pDepthAttachment = &depthAttachment;
							renderingInfo.pStencilAttachment = &detencilAttacment;

							renderDescription.DepthStencil->SetLayout(renderDescription.DepthStencilAttachment.FinalLayout);
						}

						{
							IS_PROFILE_SCOPE("vkCmdBeginRendering");
							vkCmdBeginRendering(m_commandList, &renderingInfo);
						}
					}
					else
					{
						m_dynamicRendering = false;
						RHI_Renderpass renderpass = m_context->GetRenderpassManager().GetOrCreateRenderpass(renderDescription);
						VkRenderPass vkRenderpass = reinterpret_cast<VkRenderPass>(renderpass.Resource);

						int descriptionIndex = 0;
						for (RHI_Texture* texture : renderDescription.ColourAttachments)
						{
							texture->SetLayout(renderDescription.Attachments.at(descriptionIndex++).FinalLayout);
						}

						if (renderDescription.DepthStencil)
						{
							renderDescription.DepthStencil->SetLayout(renderDescription.DepthStencilAttachment.FinalLayout);
						}

						std::vector<VkClearValue> clearColours;
						CreateFramebuffer(vkRenderpass, rect, clearColours);

						const u64 psoHash = m_pso.GetHash();
						VkRenderPassBeginInfo renderpassInfo = {};
						renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
						renderpassInfo.renderPass = vkRenderpass;
						renderpassInfo.framebuffer = m_framebuffers[psoHash];
						renderpassInfo.renderArea = rect;
						renderpassInfo.pClearValues = clearColours.data();
						renderpassInfo.clearValueCount = static_cast<u32>(clearColours.size());

						vkCmdBeginRenderPass(m_commandList, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
					}
					m_activeRenderpass = true;
				}
			}

			void RHI_CommandList_Vulkan::EndRenderpass()
			{
				IS_PROFILE_FUNCTION();
				if (m_activeRenderpass)
				{
					if (m_dynamicRendering)
					{
						vkCmdEndRendering(m_commandList);
					}
					else
					{
						vkCmdEndRenderPass(m_commandList);
					}
					m_activeRenderpass = false;
				}
			}

			void RHI_CommandList_Vulkan::SetPipeline(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();
				m_pso = pso;
			}

			void RHI_CommandList_Vulkan::SetPushConstant(u32 offset, u32 size, const void* data)
			{
				PushConstant pc = m_pso.Shader->GetPushConstant();
				vkCmdPushConstants(m_commandList, m_bound_pipeline_layout, ShaderStageFlagsToVulkan(pc.ShaderStages), offset, size, data);
			}

			void RHI_CommandList_Vulkan::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y)
			{
				IS_PROFILE_FUNCTION();
				std::array<VkViewport, 1> viewports = { VkViewport { x, y, width, height, minDepth, maxDepth } };
				if (invert_y)
				{
					viewports = { VkViewport { x, height - y, width, -height, minDepth, maxDepth } };
				}
				vkCmdSetViewport(m_commandList, 0, static_cast<u32>(viewports.size()), viewports.data());
				m_drawData.Viewport.x = width;
				m_drawData.Viewport.y = height;
			}

			void RHI_CommandList_Vulkan::SetScissor(int x, int y, int width, int height)
			{
				IS_PROFILE_FUNCTION();
				std::array<VkRect2D, 1> scissors = 
				{ 
					VkRect2D{ VkOffset2D{ x, y },
					VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) } }
			};
				vkCmdSetScissor(m_commandList, 0, static_cast<u32>(scissors.size()), scissors.data());
				m_drawData.Siccsior.x = width;
				m_drawData.Siccsior.y = height;
			}

			void RHI_CommandList_Vulkan::SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor)
			{
				vkCmdSetDepthBias(m_commandList, depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);
			}

			void RHI_CommandList_Vulkan::SetLineWidth(float width)
			{
				vkCmdSetLineWidth(m_commandList, width);
			}

			void RHI_CommandList_Vulkan::SetVertexBuffer(RHI_Buffer* buffer)
			{
				IS_PROFILE_FUNCTION();

				if (buffer == m_bound_vertex_buffer)
				{
					return;
				}

				m_bound_vertex_buffer = buffer;
				const RHI_Buffer_Vulkan* bufferVulkan = static_cast<RHI_Buffer_Vulkan*>(buffer);
				std::array<VkBuffer, 1> buffers = { bufferVulkan->GetBuffer() };
				std::array<VkDeviceSize, 1> offsets = { 0 };
				{
					IS_PROFILE_SCOPE("bindVertexBuffers");
					vkCmdBindVertexBuffers(m_commandList, 0, static_cast<u32>(buffers.size()), buffers.data(), offsets.data());
					RenderStats::Instance().VertexBufferBindings++;
					m_context->GetResourceRenderTracker().TrackResource(buffer);
				}
			}

			void RHI_CommandList_Vulkan::SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type)
			{
				IS_PROFILE_FUNCTION();

				if (buffer == m_bound_index_buffer)
				{
					return;
				}

				m_bound_index_buffer = buffer;
				const RHI_Buffer_Vulkan* bufferVulkan = static_cast<RHI_Buffer_Vulkan*>(buffer);
				vkCmdBindIndexBuffer(m_commandList, bufferVulkan->GetBuffer(), 0, IndexTypeToVulkan(index_type));
				RenderStats::Instance().IndexBufferBindings++;
				m_context->GetResourceRenderTracker().TrackResource(buffer);
			}

			void RHI_CommandList_Vulkan::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw())
				{
					vkCmdDraw(m_commandList, vertexCount, instanceCount, firstVertex, firstInstance);
					RenderStats::Instance().DrawCalls++;
				}
			}

			void RHI_CommandList_Vulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw())
				{
					{
						IS_PROFILE_SCOPE("API call");
						vkCmdDrawIndexed(m_commandList, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
						RenderStats::Instance().DrawIndexedCalls++;
						RenderStats::Instance().DrawIndexedIndicesCount += indexCount;
					}
				}
			}

			void RHI_CommandList_Vulkan::BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout)
			{
				IS_PROFILE_FUNCTION();
				///ASSERT_MSG(m_, "[RHI_CommandList_Vulkan::BindPipeline] Must be in an active renderpass.");

				m_pso = pso;
				m_activePSO = pso;

				VkPipeline pipelineVk;
				{
					IS_PROFILE_SCOPE("GetOrCreatePSO");
					RHI_Pipeline* pipeline = m_context_vulkan->GetPipelineManager().GetOrCreatePSO(m_pso);
					pipelineVk = static_cast<RHI_Pipeline_Vulkan*>(pipeline)->GetPipeline();
				}
				{
					IS_PROFILE_SCOPE("Get pipeline layout");
					RHI_PipelineLayout* layout = m_context_vulkan->GetPipelineLayoutManager().GetOrCreateLayout(m_pso);
					m_bound_pipeline_layout = static_cast<RHI_PipelineLayout_Vulkan*>(layout)->GetPipelineLayout();
				}
				vkCmdBindPipeline(m_commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineVk);
				m_descriptorAllocator->SetPipeline(pso);
			}

			void RHI_CommandList_Vulkan::BeginTimeBlock(const std::string& blockName)
			{
				BeginTimeBlock(blockName, glm::vec4(1, 1, 1, 1));
			}

			void RHI_CommandList_Vulkan::BeginTimeBlock(const std::string& blockName, glm::vec4 colour)
			{
				ASSERT(m_state == RHI_CommandListStates::Recording);
				ASSERT_MSG(m_activeDebugUtilsLabel.pLabelName == VkDebugUtilsLabelEXT().pLabelName, "[RHI_CommandList_Vulkan::BeginTimeBlock] Time block must be ended before a new one can start.");

				VkDebugUtilsLabelEXT label = {};
				label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				label.pLabelName = blockName.c_str();
				label.color[0] = colour[0];
				label.color[1] = colour[1];
				label.color[2] = colour[2];
				label.color[3] = colour[3];

				m_activeDebugUtilsLabel = label;
				if (m_cmdBeginDebugUtilsLabelEXT)
				{
					m_cmdBeginDebugUtilsLabelEXT(m_commandList, &m_activeDebugUtilsLabel);
				}
			}

			void RHI_CommandList_Vulkan::EndTimeBlock()
			{
				if (m_cmdEndDebugUtilsLabelEXT)
				{
					m_cmdEndDebugUtilsLabelEXT(m_commandList);
				}
				m_activeDebugUtilsLabel = VkDebugUtilsLabelEXT();
			}

			bool RHI_CommandList_Vulkan::BindDescriptorSets()
			{
				IS_PROFILE_FUNCTION();

				std::vector<RHI_DescriptorSet*> descriptorSets;
				bool result = m_descriptorAllocator->GetDescriptorSets(descriptorSets);
				u64 hash = 0;
				for (const auto& s : descriptorSets)
				{
					HashCombine(hash, s);
				}

				if (descriptorSets.size() > 0)// && m_boundDescriptors != hash)
				{
					//m_boundDescriptors = hash;
	
					std::vector<VkDescriptorSet> sets;
					sets.reserve(descriptorSets.size());
					{
						IS_PROFILE_SCOPE("reinterpret_cast to vulkan");
						for (const auto& s : descriptorSets)
						{
							sets.push_back(reinterpret_cast<VkDescriptorSet>(s->GetResource()));
						}
					}

					std::vector<u32> dynamicOffsets = m_descriptorAllocator->GetDynamicOffsets();
					{
						IS_PROFILE_SCOPE("API call");
						vkCmdBindDescriptorSets(m_commandList, VK_PIPELINE_BIND_POINT_GRAPHICS, m_bound_pipeline_layout, 0, 
							static_cast<u32>(sets.size()), sets.data(), static_cast<u32>(dynamicOffsets.size()), dynamicOffsets.data());
						RenderStats::Instance().DescriptorSetBindings++;
						RenderStats::Instance().DescriptorSetUsedCount += static_cast<u32>(sets.size());
					}
				}
				return result;
			}

			void RHI_CommandList_Vulkan::SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout)
			{
				ImageLayout image_layout = texture->GetLayout();
				Graphics::PipelineBarrier pipeline_barrier = { };

				ImageBarrier image_barrer = {};
				image_barrer.SrcAccessFlags = ImageLayoutToAccessMask(image_layout);
				image_barrer.DstAccessFlags = ImageLayoutToAccessMask(layout);
				image_barrer.OldLayout = image_layout;
				image_barrer.NewLayout = layout;
				image_barrer.Image = texture;
				image_barrer.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(PixelFormatToAspectFlags(texture->GetFormat()));

				if (image_layout == ImageLayout::PresentSrc)
				{
					pipeline_barrier.SrcStage = +PipelineStageFlagBits::BottomOfPipe;
				}
				else if (image_layout == ImageLayout::Undefined)
				{
					pipeline_barrier.SrcStage = +PipelineStageFlagBits::TopOfPipe;
				}
				else
				{
					pipeline_barrier.SrcStage = AccessFlagBitsToPipelineStageFlag(image_barrer.SrcAccessFlags);
				}

				if (image_layout == ImageLayout::PresentSrc)
				{
					pipeline_barrier.DstStage = +PipelineStageFlagBits::TopOfPipe;
				}
				else
				{
					pipeline_barrier.DstStage = AccessFlagBitsToPipelineStageFlag(image_barrer.DstAccessFlags);
				}

				pipeline_barrier.ImageBarriers.push_back(image_barrer);
				PipelineBarrier(pipeline_barrier);
			}

			void RHI_CommandList_Vulkan::CreateFramebuffer(VkRenderPass renderpass, VkRect2D rect, std::vector<VkClearValue>& clearColours)
			{
				const u64 psoHash = m_pso.GetHash();
				if (m_framebuffers.find(psoHash) != m_framebuffers.end())
				{
					return;
				}

				std::vector<VkImageView> imageViews;
				if (m_pso.Swapchain)
				{
					VkSwapchainKHR swapchainVulkan = m_context_vulkan->GetSwapchain();
					imageViews.push_back(m_context_vulkan->GetSwapchainImageView());

					VkClearValue clearValue;
					clearValue.color.float32[0] = 0;
					clearValue.color.float32[1] = 0;
					clearValue.color.float32[2] = 0;
					clearValue.color.float32[3] = 1;
					clearColours.push_back(clearValue);
				}
				else
				{
					for (RHI_Texture* rt : m_pso.RenderTargets)
					{
						if (rt)
						{
							RHI_Texture_Vulkan* textureVulkan = static_cast<RHI_Texture_Vulkan*>(rt);
							imageViews.push_back(textureVulkan->GetImageView());

							VkClearValue clearValue;
							clearValue.color.float32[0] = 0;
							clearValue.color.float32[1] = 0;
							clearValue.color.float32[2] = 0;
							clearValue.color.float32[3] = 1;
							clearColours.push_back(clearValue);
						}
					}
				}

				if (m_pso.DepthStencil)
				{
					RHI_Texture_Vulkan* depthVulkan = static_cast<RHI_Texture_Vulkan*>(m_pso.DepthStencil);
					imageViews.push_back(depthVulkan->GetImageView());

					VkClearValue clearValue;
					clearValue.depthStencil.depth = m_pso.DepthSteniclClearValue.x;
					clearValue.depthStencil.stencil = static_cast<u32>(m_pso.DepthSteniclClearValue.y);
					clearColours.push_back(clearValue);
				}

				VkFramebufferCreateInfo frameBufferInfo = {};
				frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				frameBufferInfo.renderPass = renderpass;
				frameBufferInfo.pAttachments = imageViews.data();
				frameBufferInfo.attachmentCount = static_cast<u32>(imageViews.size());
				frameBufferInfo.width = rect.extent.width;
				frameBufferInfo.height = rect.extent.height;
				frameBufferInfo.layers = 1;

				VkFramebuffer framebuffer;
				ThrowIfFailed(vkCreateFramebuffer(m_context_vulkan->GetDevice(), &frameBufferInfo, nullptr, &framebuffer));
				m_framebuffers[psoHash] = framebuffer;

				m_context_vulkan->SetObjectName("Framebuffer " + std::to_string(psoHash), (u64)framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER);
			}


			//// <summary>
			//// RHI_CommandListAllocator_Vulkan
			//// </summary>
			//// <param name="context"></param>
			void RHI_CommandListAllocator_Vulkan::Create(RenderContext* context)
			{
				std::lock_guard lock(m_lock);
				m_context = static_cast<RenderContext_Vulkan*>(context);

				VkCommandPoolCreateInfo poolCreateInfo = {};
				poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				ThrowIfFailed(vkCreateCommandPool(m_context->GetDevice(), &poolCreateInfo, nullptr, &m_allocator));
			}

			RHI_CommandList* RHI_CommandListAllocator_Vulkan::GetCommandList()
			{
				std::lock_guard lock(m_lock);
				if (m_freeLists.size() > 0)
				{
					RHI_CommandList* list = *m_freeLists.begin();
					m_freeLists.erase(m_freeLists.begin());
					m_allocLists.insert(list);
					list->Reset();

					RHI_CommandList_Vulkan* cmdListVulkan = static_cast<RHI_CommandList_Vulkan*>(list);
					VkCommandBufferBeginInfo beginInfo = {};
					beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
					ThrowIfFailed(vkBeginCommandBuffer(cmdListVulkan->GetCommandList(), &beginInfo));
					cmdListVulkan->m_state = RHI_CommandListStates::Recording;

					return list;
				}

				VkCommandBufferAllocateInfo bufferAllocateInfo = {};
				bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				bufferAllocateInfo.commandBufferCount = 1;
				bufferAllocateInfo.commandPool = m_allocator;
				bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

				RHI_CommandList_Vulkan* list = static_cast<RHI_CommandList_Vulkan*>(RHI_CommandList::New());
				list->Create(m_context);
				list->m_allocator = this;
				vkAllocateCommandBuffers(m_context->GetDevice(), &bufferAllocateInfo, &list->m_commandList);
				
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				vkBeginCommandBuffer(list->GetCommandList(), &beginInfo);

				list->m_state = RHI_CommandListStates::Recording;
				m_allocLists.insert(list);
				list->SetName("CommandList");
				return list;
			}

			RHI_CommandList* RHI_CommandListAllocator_Vulkan::GetSingleSubmitCommandList()
			{
				return GetCommandList();
			}

			void RHI_CommandListAllocator_Vulkan::ReturnSingleSubmitCommandList(RHI_CommandList* cmdList)
			{
				return ReturnCommandList(cmdList);
			}

			void RHI_CommandListAllocator_Vulkan::Reset()
			{
				vkResetCommandPool(m_context->GetDevice(), m_allocator, 0);
				while (m_allocLists.size() > 0)
				{
					RHI_CommandList* list = *m_allocLists.begin();
					list->Reset();

					m_freeLists.insert(list);
					m_allocLists.erase(list);
				}
			}

			void RHI_CommandListAllocator_Vulkan::Release()
			{
				std::lock_guard lock(m_lock);

				if (m_allocator)
				{
					Reset();

					for (auto list : m_allocLists)
					{
						list->Release();
						DeleteTracked(list);
					}
					for (auto list : m_freeLists)
					{
						list->Release();
						DeleteTracked(list);
					}
					m_allocLists.clear();
					m_freeLists.clear();

					vkDestroyCommandPool(m_context->GetDevice(), m_allocator, nullptr);
					m_allocator = nullptr;
				}
			}

			bool RHI_CommandListAllocator_Vulkan::ValidResource()
			{
				return m_allocator;
			}

			void RHI_CommandListAllocator_Vulkan::SetName(std::string name)
			{
				m_context->SetObjectName(name, (u64)m_allocator, VK_OBJECT_TYPE_COMMAND_POOL);
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)