#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
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
			}

			void RHI_CommandList_Vulkan::PipelineBarrier(Graphics::PipelineBarrier barrier)
			{
				std::vector<vk::BufferMemoryBarrier> bufferBarriers;
				for (const BufferBarrier& bufferBarrier : barrier.BufferBarriers)
				{
					vk::BufferMemoryBarrier b = { };
					b.srcAccessMask = AccessFlagsToVulkan(bufferBarrier.SrcAccessFlags);
					b.dstAccessMask = AccessFlagsToVulkan(bufferBarrier.DstAccessFlags);
					b.buffer = static_cast<RHI_Buffer_Vulkan*>(bufferBarrier.Buffer)->GetBuffer();
					b.offset = bufferBarrier.Offset;
					b.size = bufferBarrier.Size;

					bufferBarriers.push_back(std::move(b));
				}

				std::vector<vk::ImageMemoryBarrier> imageBarriers;
				for (const ImageBarrier& imageBarrier : barrier.ImageBarriers)
				{
					vk::ImageMemoryBarrier b = { };
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
				, std::vector<vk::BufferMemoryBarrier> const& bufferMemoryBarrier, std::vector<vk::ImageMemoryBarrier> const& imageMemoryBarrier)
			{
				m_commandList.pipelineBarrier(
					PipelineStageFlagsToVulkan(srcStage), 
					PipelineStageFlagsToVulkan(dstStage), 
					vk::DependencyFlagBits::eByRegion, 
					{ },
					bufferMemoryBarrier,
					imageMemoryBarrier);
				RenderStats::Instance().PipelineBarriers++;
			}

			void RHI_CommandList_Vulkan::PipelineBarrierBuffer(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<vk::BufferMemoryBarrier> const& bufferMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, bufferMemoryBarrier, { });
			}

			void RHI_CommandList_Vulkan::PipelineBarrierImage(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<vk::ImageMemoryBarrier> const& imageMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, { }, imageMemoryBarrier);
			}

			void RHI_CommandList_Vulkan::Reset()
			{
				RHI_CommandList::Reset();
				for (auto& pair : m_framebuffers)
				{
					m_context_vulkan->GetDevice().destroyFramebuffer(pair.second);
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
					m_commandList.end();
					m_state = RHI_CommandListStates::Ended;
				}
			}

			void RHI_CommandList_Vulkan::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Buffer_Vulkan* dstVulkan = static_cast<RHI_Buffer_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = static_cast<RHI_Buffer_Vulkan*>(src);
				std::array<vk::BufferCopy, 1> copyRegion =
				{
					vk::BufferCopy(0, offset, src->GetSize())
				};
				m_commandList.copyBuffer(srcVulkan->GetBuffer(), dstVulkan->GetBuffer(), copyRegion);
			}

			void RHI_CommandList_Vulkan::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src)
			{
				RHI_Texture_Vulkan* dstVulkan = static_cast<RHI_Texture_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = static_cast<RHI_Buffer_Vulkan*>(src);
				std::array<vk::BufferImageCopy, 1> copyRegion =
				{
					vk::BufferImageCopy(
						0,
						0,
						0,
						vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
						vk::Offset3D(0, 0, 0),
						vk::Extent3D(dst->GetWidth(), dst->GetHeight(), 1))
				};
				vk::ImageMemoryBarrier memoryBarriers = vk::ImageMemoryBarrier(
					vk::AccessFlagBits::eNoneKHR,
					vk::AccessFlagBits::eTransferWrite,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eTransferDstOptimal,
					m_context_vulkan->GetFamilyQueueIndex(GPUQueue_Graphics),
					m_context_vulkan->GetFamilyQueueIndex(GPUQueue_Graphics),
					dstVulkan->GetImage(),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

				PipelineBarrierImage(+PipelineStageFlagBits::TopOfPipe, +PipelineStageFlagBits::Transfer, { memoryBarriers });
				m_commandList.copyBufferToImage(srcVulkan->GetBuffer(), dstVulkan->GetImage(), vk::ImageLayout::eTransferDstOptimal, copyRegion);

				memoryBarriers.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				memoryBarriers.dstAccessMask = vk::AccessFlagBits::eShaderRead;
				memoryBarriers.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				memoryBarriers.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				PipelineBarrierImage(+PipelineStageFlagBits::Transfer, +PipelineStageFlagBits::FragmentShader, { memoryBarriers });
			}

			void RHI_CommandList_Vulkan::Release()
			{
				if (m_commandList)
				{
					for (auto& pair : m_framebuffers)
					{
						m_context_vulkan->GetDevice().destroyFramebuffer(pair.second);
					}
					m_framebuffers.clear();

					m_context_vulkan->GetDevice().freeCommandBuffers(m_allocator->GetAllocator(), { m_commandList });
					m_commandList = nullptr;
				}

			}

			bool RHI_CommandList_Vulkan::ValidResouce()
			{
				return m_commandList;
			}

			void RHI_CommandList_Vulkan::SetName(std::wstring name)
			{
				m_context_vulkan->SetObejctName(name, (u64)m_commandList.operator VkCommandBuffer(), m_commandList.objectType);
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

					///		vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier(
					///			vk::AccessFlagBits::eNoneKHR,
					///			vk::AccessFlagBits::eColorAttachmentWrite,
					///			vk::ImageLayout::eUndefined,
					///			vk::ImageLayout::eColorAttachmentOptimal,
					///			RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					///			RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					///			textureVulkan->GetImage(),
					///			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
					///		PipelineBarrierImage(PipelineStageFlagBits::TopOfPipe, PipelineStageFlagBits::ColourAttachmentOutput, { barrier });
					///	}
					///}

					vk::Rect2D rect = vk::Rect2D({ }, { (u32)m_drawData.Viewport.x, (u32)m_drawData.Viewport.y });

					if (m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
					{
						std::vector<vk::RenderingAttachmentInfo> colourAttachments;
						vk::RenderingAttachmentInfo depthAttachment;
						vk::RenderingAttachmentInfo detencilAttacment;

						auto makeRenderingAttachment = 
						[](const RHI_Texture* texture, const AttachmentDescription& attachment_description) -> vk::RenderingAttachmentInfo
						{
							std::array<float, 4> clearColourValues = {
								attachment_description.ClearColour.x
								, attachment_description.ClearColour.y
								, attachment_description.ClearColour.z
								, attachment_description.ClearColour.w };


							vk::RenderingAttachmentInfo attachment = { };
							attachment.setImageView(static_cast<const RHI_Texture_Vulkan*>(texture)->GetImageView(attachment_description.Layer_Array_Index));
							attachment.imageLayout = ImageLayoutToVulkan(attachment_description.FinalLayout);
							attachment.loadOp = AttachmentLoadOpToVulkan(attachment_description.LoadOp);
							attachment.storeOp = vk::AttachmentStoreOp::eStore;
							attachment.clearValue.color = vk::ClearColorValue(clearColourValues);

							attachment.imageLayout = attachment.imageLayout == vk::ImageLayout::ePresentSrcKHR ?
								vk::ImageLayout::eColorAttachmentOptimal : attachment.imageLayout;

							return attachment;
						};

						vk::RenderingInfo renderingInfo = { };
						renderingInfo.renderArea = rect;
						renderingInfo.layerCount = 1;

						int descriptionIndex = 0;
						for (RHI_Texture* texture : renderDescription.ColourAttachments)
						{
							vk::RenderingAttachmentInfo attachment = makeRenderingAttachment(texture, renderDescription.Attachments.at(descriptionIndex));
							colourAttachments.push_back(attachment);
							texture->SetLayout(renderDescription.Attachments.at(descriptionIndex).FinalLayout);
							++descriptionIndex;
						}
						renderingInfo.setColorAttachments(colourAttachments);

						if (renderDescription.DepthStencil)
						{
							depthAttachment = makeRenderingAttachment(renderDescription.DepthStencil, renderDescription.DepthStencilAttachment);
							depthAttachment.clearValue = { };
							depthAttachment.clearValue.depthStencil = vk::ClearDepthStencilValue(
								renderDescription.DepthStencilAttachment.DepthStencilClear.x, 
								static_cast<u32>(renderDescription.DepthStencilAttachment.DepthStencilClear.y));
							renderingInfo.setPDepthAttachment(&depthAttachment);
							renderingInfo.setPStencilAttachment(&detencilAttacment);

							renderDescription.DepthStencil->SetLayout(renderDescription.DepthStencilAttachment.FinalLayout);
						}

						m_commandList.beginRendering(renderingInfo);
					}
					else
					{
						RHI_Renderpass renderpass = m_context->GetRenderpassManager().GetOrCreateRenderpass(renderDescription);
						vk::RenderPass vkRenderpass = *reinterpret_cast<vk::RenderPass*>(&renderpass);

						std::vector<vk::ClearValue> clearColours;
						CreateFramebuffer(vkRenderpass, rect, clearColours);

						const u64 psoHash = m_pso.GetHash();
						vk::RenderPassBeginInfo info = vk::RenderPassBeginInfo(vkRenderpass, m_framebuffers[psoHash], rect, clearColours);
						m_commandList.beginRenderPass(info, vk::SubpassContents::eInline);
					}
					m_activeRenderpass = true;
				}
			}

			void RHI_CommandList_Vulkan::EndRenderpass()
			{
				IS_PROFILE_FUNCTION();
				if (m_activeRenderpass)
				{
					if (m_context->IsExtensionEnabled(DeviceExtension::VulkanDynamicRendering))
					{
						m_commandList.endRendering();
					}
					else
					{
						m_commandList.endRenderPass();
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
				m_commandList.pushConstants(m_bound_pipeline_layout, ShaderStageFlagsToVulkan(pc.ShaderStages), offset, size, data);
			}

			void RHI_CommandList_Vulkan::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y)
			{
				IS_PROFILE_FUNCTION();
				std::array<vk::Viewport, 1> viewports = { vk::Viewport(x, y, width, height, minDepth, maxDepth) };
				if (invert_y)
				{
					viewports = { vk::Viewport(x, height - y, width, -height, minDepth, maxDepth) };
				}
				m_commandList.setViewport(0, viewports);
				m_drawData.Viewport.x = width;
				m_drawData.Viewport.y = height;
			}

			void RHI_CommandList_Vulkan::SetScissor(int x, int y, int width, int height)
			{
				IS_PROFILE_FUNCTION();
				std::array<vk::Rect2D, 1> scissors = { vk::Rect2D(vk::Offset2D(x, y),
					vk::Extent2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height))) };
				m_commandList.setScissor(0, scissors);
				m_drawData.Siccsior.x = width;
				m_drawData.Siccsior.y = height;
			}

			void RHI_CommandList_Vulkan::SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor)
			{
				m_commandList.setDepthBias(depth_bias_constant_factor, depth_bias_clamp, depth_bias_slope_factor);
			}

			void RHI_CommandList_Vulkan::SetLineWidth(float width)
			{
				m_commandList.setLineWidth(width);
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
				std::array<vk::Buffer, 1> buffers = { bufferVulkan->GetBuffer() };
				std::array<vk::DeviceSize, 1> offsets = { 0 };
				{
					IS_PROFILE_SCOPE("bindVertexBuffers");
					m_commandList.bindVertexBuffers(0, buffers, offsets);
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
				m_commandList.bindIndexBuffer(bufferVulkan->GetBuffer(), 0, IndexTypeToVulkan(index_type));
				RenderStats::Instance().IndexBufferBindings++;
				m_context->GetResourceRenderTracker().TrackResource(buffer);
			}

			void RHI_CommandList_Vulkan::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw())
				{
					m_commandList.draw(vertexCount, instanceCount, firstVertex, firstInstance);
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
						m_commandList.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
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

				vk::Pipeline pipeline;
				{
					IS_PROFILE_SCOPE("GetOrCreatePSO");
					pipeline = m_context_vulkan->GetPipelineStateObjectManager().GetOrCreatePSO(m_pso);
				}
				m_bound_pipeline_layout = m_context_vulkan->GetPipelineLayoutManager().GetOrCreateLayout(m_pso);
				m_commandList.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
				m_descriptorAllocator->SetPipeline(pso);
			}

			void RHI_CommandList_Vulkan::BeginTimeBlock(const char* block_name)
			{
				m_context_vulkan->BeginDebugMarker(block_name);
			}

			void RHI_CommandList_Vulkan::EndTimeBlock()
			{

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

					
					std::vector<vk::DescriptorSet> sets;
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
						m_commandList.bindDescriptorSets(vk::PipelineBindPoint::eGraphics
							, m_bound_pipeline_layout
							, 0
							, sets
							, dynamicOffsets);
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

				ImageBarrier image_barrer;
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

			void RHI_CommandList_Vulkan::CreateFramebuffer(vk::RenderPass renderpass, vk::Rect2D rect, std::vector<vk::ClearValue>& clearColours)
			{
				const u64 psoHash = m_pso.GetHash();
				if (m_framebuffers.find(psoHash) != m_framebuffers.end())
				{
					return;
				}

				std::vector<vk::ImageView> imageViews;
				if (m_pso.Swapchain)
				{
					vk::SwapchainKHR swapchainVulkan = m_context_vulkan->GetSwapchain();
					imageViews.push_back(m_context_vulkan->GetSwapchainImageView());

					vk::ClearValue clearValue;
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

							vk::ClearValue clearValue;
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

					vk::ClearDepthStencilValue clearValue;
					clearValue.depth = m_pso.DepthSteniclClearValue.x;
					clearValue.stencil = static_cast<u32>(m_pso.DepthSteniclClearValue.y);
					clearColours.push_back(clearValue);
				}

				vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({}, renderpass, imageViews, rect.extent.width, rect.extent.height, 1);
				m_framebuffers[psoHash] = m_context_vulkan->GetDevice().createFramebuffer(frameBufferInfo);
				m_context_vulkan->SetObejctName(L"Framebuffer " + std::to_wstring(psoHash), (u64)m_framebuffers[psoHash].operator VkFramebuffer(), vk::ObjectType::eFramebuffer);
			}


			//// <summary>
			//// RHI_CommandListAllocator_Vulkan
			//// </summary>
			//// <param name="context"></param>
			void RHI_CommandListAllocator_Vulkan::Create(RenderContext* context)
			{
				std::lock_guard lock(m_lock);
				m_context = static_cast<RenderContext_Vulkan*>(context);

				vk::CommandPoolCreateInfo poolCreateInfo = vk::CommandPoolCreateInfo();
				poolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
				m_allocator = m_context->GetDevice().createCommandPool(poolCreateInfo);
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
					cmdListVulkan->GetCommandList().begin(vk::CommandBufferBeginInfo());
					cmdListVulkan->m_state = RHI_CommandListStates::Recording;

					return list;
				}

				vk::CommandBufferAllocateInfo info = vk::CommandBufferAllocateInfo(m_allocator);
				info.setCommandBufferCount(1);

				RHI_CommandList_Vulkan* list = static_cast<RHI_CommandList_Vulkan*>(RHI_CommandList::New());
				list->Create(m_context);
				list->m_allocator = this;
				list->m_commandList = m_context->GetDevice().allocateCommandBuffers(info)[0];
				
				list->GetCommandList().begin(vk::CommandBufferBeginInfo());
				m_allocLists.insert(list);
				list->SetName(L"CommandList");
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
				m_context->GetDevice().resetCommandPool(m_allocator);
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

					m_context->GetDevice().destroyCommandPool(m_allocator);
					m_allocator = nullptr;
				}
			}

			bool RHI_CommandListAllocator_Vulkan::ValidResouce()
			{
				return m_allocator;
			}

			void RHI_CommandListAllocator_Vulkan::SetName(std::wstring name)
			{
				m_context->SetObejctName(name, (u64)m_allocator.operator VkCommandPool(), m_allocator.objectType);
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)