#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/Window.h"

#include "Graphics/RenderTarget.h"

#include "optick.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			void RHI_CommandList_Vulkan::PipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage, Core::Slice<vk::BufferMemoryBarrier> bufferMemoryBarrier, Core::Slice<vk::ImageMemoryBarrier> imageMemoryBarrier)
			{
				vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemory(bufferMemoryBarrier.GetSize(), bufferMemoryBarrier.GetBegin());
				vk::ArrayProxy<const vk::ImageMemoryBarrier>imageMemory(imageMemoryBarrier.GetSize(), imageMemoryBarrier.GetBegin());
				m_commandList.pipelineBarrier(
					PipelineStageFlagsToVulkan(srcStage), 
					PipelineStageFlagsToVulkan(dstStage), 
					vk::DependencyFlagBits::eByRegion, 
					{ },
					bufferMemory,
					imageMemory);
			}

			void RHI_CommandList_Vulkan::PipelineBarrierBuffer(PipelineStageFlags srcStage, PipelineStageFlags dstStage, Core::Slice<vk::BufferMemoryBarrier> bufferMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, bufferMemoryBarrier, { });
			}

			void RHI_CommandList_Vulkan::PipelineBarrierImage(PipelineStageFlags srcStage, PipelineStageFlags dstStage, Core::Slice<vk::ImageMemoryBarrier> imageMemoryBarrier)
			{
				PipelineBarrier(srcStage, dstStage, { }, imageMemoryBarrier);
			}

			/// <summary>
			/// RHI_CommandList_Vulkan
			/// </summary>
			void RHI_CommandList_Vulkan::Reset()
			{
				RHI_CommandList::Reset();
				for (auto& pair : m_framebuffers)
				{
					RenderContextVulkan()->GetDevice().destroyFramebuffer(pair.second);
				}
				m_framebuffers.clear();
			}

			void RHI_CommandList_Vulkan::Close()
			{
				if (m_commandList)
				{
					m_commandList.end();
				}
			}

			void RHI_CommandList_Vulkan::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Buffer_Vulkan* dstVulkan = dynamic_cast<RHI_Buffer_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = dynamic_cast<RHI_Buffer_Vulkan*>(src);
				std::array<vk::BufferCopy, 1> copyRegion =
				{
					vk::BufferCopy(0, offset, src->GetSize())
				};
				m_commandList.copyBuffer(srcVulkan->GetBuffer(), dstVulkan->GetBuffer(), copyRegion);
			}

			void RHI_CommandList_Vulkan::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src)
			{
				RHI_Texture_Vulkan* dstVulkan = dynamic_cast<RHI_Texture_Vulkan*>(dst);
				RHI_Buffer_Vulkan* srcVulkan = dynamic_cast<RHI_Buffer_Vulkan*>(src);
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
					RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
					dstVulkan->GetImage(),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

				PipelineBarrierImage(PipelineStageFlagBits::TopOfPipe, PipelineStageFlagBits::Transfer, { memoryBarriers });
				m_commandList.copyBufferToImage(srcVulkan->GetBuffer(), dstVulkan->GetImage(), vk::ImageLayout::eTransferDstOptimal, copyRegion);

				memoryBarriers.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				memoryBarriers.dstAccessMask = vk::AccessFlagBits::eShaderRead;
				memoryBarriers.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				memoryBarriers.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				PipelineBarrierImage(PipelineStageFlagBits::Transfer, PipelineStageFlagBits::FragmentShader, { memoryBarriers });
			}

			void RHI_CommandList_Vulkan::Release()
			{
				if (m_commandList)
				{
					for (auto& pair : m_framebuffers)
					{
						RenderContextVulkan()->GetDevice().destroyFramebuffer(pair.second);
					}
					m_framebuffers.clear();

					RenderContextVulkan()->GetDevice().freeCommandBuffers(m_allocator->GetAllocator(), { m_commandList });
					m_commandList = nullptr;
				}

			}
			bool RHI_CommandList_Vulkan::ValidResouce()
			{
				return m_commandList;
			}

			void RHI_CommandList_Vulkan::SetName(std::wstring name)
			{
				RenderContextVulkan()->SetObejctName(name, (u64)m_commandList.operator VkCommandBuffer(), m_commandList.objectType);
			}

			void RHI_CommandList_Vulkan::SetPipeline(PipelineStateObject pso)
			{
				OPTICK_EVENT();
				m_pso = pso;
				FrameResourceVulkan()->DescriptorAllocator.SetPipeline(m_pso);
			}

			void RHI_CommandList_Vulkan::SetUniform(int set, int binding, DescriptorBufferView view)
			{
				OPTICK_EVENT();
				FrameResourceVulkan()->DescriptorAllocator.SetUniform(set, binding, FrameResourceVulkan()->UniformBuffer.GetView(view.Offset, view.SizeInBytes));
			}

			void RHI::Vulkan::RHI_CommandList_Vulkan::SetTexture(int set, int binding, RHI_Texture* texture)
			{
				OPTICK_EVENT();
				FrameResourceVulkan()->DescriptorAllocator.SetTexture(set, binding, texture);
			}

			void RHI_CommandList_Vulkan::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
			{
				OPTICK_EVENT();
				std::array<vk::Viewport, 1> viewports = { vk::Viewport(x, height - y, width, -height, minDepth, maxDepth) };
				m_commandList.setViewport(0, viewports);
			}

			void RHI_CommandList_Vulkan::SetScissor(int x, int y, int width, int height)
			{
				OPTICK_EVENT();
				std::array<vk::Rect2D, 1> scissors = { vk::Rect2D(vk::Offset2D(x, y),
					vk::Extent2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height))) };
				m_commandList.setScissor(0, scissors);
			}

			void RHI_CommandList_Vulkan::SetVertexBuffer(RHI_Buffer* buffer)
			{
				OPTICK_EVENT();
				const RHI_Buffer_Vulkan* bufferVulkan = dynamic_cast<RHI_Buffer_Vulkan*>(buffer);
				std::array<vk::Buffer, 1> buffers = { bufferVulkan->GetBuffer() };
				std::array<vk::DeviceSize, 1> offsets = { 0 };
				m_commandList.bindVertexBuffers(0, buffers, offsets);
			}

			void RHI_CommandList_Vulkan::SetIndexBuffer(RHI_Buffer* buffer)
			{
				OPTICK_EVENT();
				const RHI_Buffer_Vulkan* bufferVulkan = dynamic_cast<RHI_Buffer_Vulkan*>(buffer);
				m_commandList.bindIndexBuffer(bufferVulkan->GetBuffer(), 0, vk::IndexType::eUint32);
			}

			void RHI_CommandList_Vulkan::Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance)
			{
				OPTICK_EVENT();
				m_commandList.draw(vertexCount, instanceCount, firstVertex, firstInstance);
			}

			void RHI_CommandList_Vulkan::DrawIndexed(int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance)
			{
				OPTICK_EVENT();
				m_commandList.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
			}

			void RHI_CommandList_Vulkan::BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout)
			{
				OPTICK_EVENT();
				u64 psoHash = pso.GetHash();
				assert(m_framebuffers.find(psoHash) == m_framebuffers.end());

				if (m_activeRenderpass && m_framebuffers.find(psoHash) == m_framebuffers.end())
				{
					m_commandList.endRenderPass();

					// After rendering everything, make sure all our RenderTargets are shader read for if they are read from.
					for (size_t i = 0; i < pso.RenderTargets.size(); ++i)
					{
						const RenderTarget* rt = pso.RenderTargets.at(i);
						if (rt)
						{
							const RHI_Texture_Vulkan* textureVulkan = static_cast<const RHI_Texture_Vulkan*>(rt->GetTexture());

							vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier(
								vk::AccessFlagBits::eColorAttachmentWrite,
								vk::AccessFlagBits::eShaderRead,
								vk::ImageLayout::eColorAttachmentOptimal,
								vk::ImageLayout::eShaderReadOnlyOptimal,
								RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
								RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
								textureVulkan->GetImage(),
								vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
							PipelineBarrierImage(PipelineStageFlagBits::ColourAttachmentOutput, PipelineStageFlagBits::FragmentShader, { barrier });
						}
					}

					m_activeRenderpass = false;
				}

				if (!m_activeRenderpass)
				{
					// Before rendering anything, make sure all our RenderTargets are in the correct layout.
					for (size_t i = 0; i < pso.RenderTargets.size(); ++i)
					{
						const RenderTarget* rt = pso.RenderTargets.at(i);
						if (rt)
						{
							const RHI_Texture_Vulkan* textureVulkan = static_cast<const RHI_Texture_Vulkan*>(rt->GetTexture());

							vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier(
								vk::AccessFlagBits::eShaderRead,
								vk::AccessFlagBits::eColorAttachmentWrite,
								vk::ImageLayout::eUndefined,
								vk::ImageLayout::eColorAttachmentOptimal,
								RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
								RenderContextVulkan()->GetFamilyQueueIndex(GPUQueue_Graphics),
								textureVulkan->GetImage(),
								vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
							PipelineBarrierImage(PipelineStageFlagBits::FragmentShader, PipelineStageFlagBits::ColourAttachmentOutput, { barrier });
						}
					}

					vk::Rect2D rect = vk::Rect2D({ }, { (u32)Window::Instance().GetWidth(), (u32)Window::Instance().GetHeight() });
					vk::RenderPass renderpass = RenderContextVulkan()->GetRenderpassManager().GetOrCreateRenderpass(RenderpassDesc_Vulkan{m_pso.RenderTargets, m_pso.DepthStencil});
					std::vector<vk::ImageView> imageViews;
					std::vector<vk::ClearValue> clearColours;
					if (m_pso.Swapchain)
					{
						vk::SwapchainKHR swapchainVulkan = RenderContextVulkan()->GetSwapchain();
						imageViews.push_back(RenderContextVulkan()->GetSwapchainImageView());

						vk::ClearValue clearValue;
						clearValue.color.float32[0] = 0;
						clearValue.color.float32[1] = 0;
						clearValue.color.float32[2] = 0;
						clearValue.color.float32[3] = 1;
						clearColours.push_back(clearValue);
					}
					else
					{
						for (RenderTarget* rt : pso.RenderTargets)
						{
							if (rt)
							{
								RHI_Texture_Vulkan* textureVulkan = static_cast<RHI_Texture_Vulkan*>(rt->GetTexture());
								imageViews.push_back(textureVulkan->GetImageView());

								vk::ClearValue clearValue;
								clearValue.color.float32[0] = 1;
								clearValue.color.float32[1] = 0;
								clearValue.color.float32[2] = 0;
								clearValue.color.float32[3] = 1;
								clearColours.push_back(clearValue);
							}
						}
					}

					if (pso.DepthStencil)
					{
						RHI_Texture_Vulkan* depthVulkan = static_cast<RHI_Texture_Vulkan*>(pso.DepthStencil->GetTexture());
						imageViews.push_back(depthVulkan->GetImageView());

						vk::ClearDepthStencilValue clearValue;
						clearValue.depth = 1.0f;
						clearValue.stencil = 0.0f;
						clearColours.push_back(clearValue);
					}

					vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({}, renderpass, imageViews, rect.extent.width, rect.extent.height, 1);
					m_framebuffers[psoHash] = RenderContextVulkan()->GetDevice().createFramebuffer(frameBufferInfo);

					vk::RenderPassBeginInfo info = vk::RenderPassBeginInfo(renderpass, m_framebuffers[psoHash], rect, clearColours);
					m_commandList.beginRenderPass(info, vk::SubpassContents::eInline);
					m_activeRenderpass = true;
				}

				vk::Pipeline pipeline = RenderContextVulkan()->GetPipelineStateObjectManager().GetOrCreatePSO(m_pso);
				m_commandList.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
			}

			bool RHI_CommandList_Vulkan::BindDescriptorSets()
			{
				OPTICK_EVENT();

				std::vector<RHI_Descriptor*> descriptors;
				bool result = FrameResourceVulkan()->DescriptorAllocator.GetDescriptors(descriptors);
				const int descriptorCount = static_cast<int>(descriptors.size());

				if (result && descriptorCount > 0)
				{
					vk::PipelineLayout layout = RenderContextVulkan()->GetPipelineLayoutManager().GetOrCreateLayout(m_pso);

					std::vector<vk::DescriptorSet> sets;
					sets.resize(descriptorCount);
					for (size_t i = 0; i < descriptorCount; ++i)
					{
						RHI_Descriptor_Vulkan* descriptorVulkan = dynamic_cast<RHI_Descriptor_Vulkan*>(descriptors.at(i));
						sets[i] = descriptorVulkan->GetSet();
					}

					std::vector<u32> dynamicOffsets = {};
					m_commandList.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, sets, dynamicOffsets);
				}
				return result && descriptorCount > 0;
			}

			RenderContext_Vulkan* RHI_CommandList_Vulkan::RenderContextVulkan()
			{
				OPTICK_EVENT();
				assert(m_context);
				return dynamic_cast<RenderContext_Vulkan*>(m_context);
			}

			FrameResource_Vulkan* RHI_CommandList_Vulkan::FrameResourceVulkan()
			{
				OPTICK_EVENT();
				assert(m_frameResouces);
				return static_cast<FrameResource_Vulkan*>(m_frameResouces);
			}


			/// <summary>
			/// RHI_CommandListAllocator_Vulkan
			/// </summary>
			/// <param name="context"></param>
			void RHI_CommandListAllocator_Vulkan::Create(RenderContext* context)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);

				vk::CommandPoolCreateInfo poolCreateInfo = vk::CommandPoolCreateInfo();
				m_allocator = m_context->GetDevice().createCommandPool(poolCreateInfo);
			}

			RHI_CommandList* RHI_CommandListAllocator_Vulkan::GetCommandList()
			{
				if (m_freeLists.size() > 0)
				{
					RHI_CommandList* list = *m_freeLists.begin();
					m_freeLists.erase(m_freeLists.begin());
					m_allocLists.insert(list);

					dynamic_cast<RHI_CommandList_Vulkan*>(list)->GetCommandList().begin(vk::CommandBufferBeginInfo());
					return list;
				}

				vk::CommandBufferAllocateInfo info = vk::CommandBufferAllocateInfo(m_allocator);
				info.setCommandBufferCount(1);
				RHI_CommandList_Vulkan* list = dynamic_cast<RHI_CommandList_Vulkan*>(RHI_CommandList::New());
				list->m_context = m_context;
				list->m_allocator = this;
				list->m_commandList = m_context->GetDevice().allocateCommandBuffers(info)[0];
				
				list->GetCommandList().begin(vk::CommandBufferBeginInfo());
				m_allocLists.insert(list);
				list->SetName(L"CommandList");
				return list;
			}

			RHI_CommandList* RHI::Vulkan::RHI_CommandListAllocator_Vulkan::GetSingleSubmitCommandList()
			{
				return GetCommandList();
			}

			void RHI::Vulkan::RHI_CommandListAllocator_Vulkan::ReturnSingleSubmitCommandList(RHI_CommandList* cmdList)
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
				if (m_allocator)
				{
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

			bool RHI::Vulkan::RHI_CommandListAllocator_Vulkan::ValidResouce()
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

#endif //#if defined(IS_VULKAN_ENABLED)