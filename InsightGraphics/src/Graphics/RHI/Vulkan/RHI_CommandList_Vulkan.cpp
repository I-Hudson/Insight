#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/Window.h"

#include "optick.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			/// <summary>
			/// RHI_CommandList_Vulkan
			/// </summary>
			void RHI_CommandList_Vulkan::Reset()
			{
				RHI_CommandList::Reset();
				if (m_framebuffer)
				{
					RenderContextVulkan()->GetDevice().destroyFramebuffer(m_framebuffer);
					m_framebuffer = nullptr;
				}
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

			void RHI_CommandList_Vulkan::Release()
			{
				if (m_commandList)
				{
					if (m_framebuffer)
					{
						RenderContextVulkan()->GetDevice().destroyFramebuffer(m_framebuffer);
						m_framebuffer = nullptr;
					}

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
				if (!m_activeRenderpass)
				{
					vk::Rect2D rect = vk::Rect2D({ }, { (u32)Window::Instance().GetWidth(), (u32)Window::Instance().GetHeight() });
					vk::RenderPass renderpass = RenderContextVulkan()->GetRenderpassManager().GetOrCreateRenderpass(RenderpassDesc_Vulkan{m_pso.RenderTargets});
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
						IS_CORE_ERROR("[CommandList_Vulkan::CanDraw] TODO");
					}

					vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({}, renderpass, imageViews, rect.extent.width, rect.extent.height, 1);
					m_framebuffer = RenderContextVulkan()->GetDevice().createFramebuffer(frameBufferInfo);

					vk::RenderPassBeginInfo info = vk::RenderPassBeginInfo(renderpass, m_framebuffer, rect, clearColours);
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