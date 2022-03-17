#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUPipelineStateObject_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSwapchain_Vulkan.h"
#include "Graphics/GPU/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/Window.h"
#include "Graphics/RenderTarget.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			GPUCommandList_Vulkan::GPUCommandList_Vulkan()
			{ }

			GPUCommandList_Vulkan::~GPUCommandList_Vulkan()
			{
				if (m_framebuffer)
				{
					GetDevice()->GetDevice().destroyFramebuffer(m_framebuffer);
					m_framebuffer = nullptr;
				}
			}

			void GPUCommandList_Vulkan::SetViewport(int width, int height)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::SetViewport] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Recording) { std::cout << "[GPUCommandList_Vulkan::SetViewport] CommandList is not recording.\n"; return; }

				vk::Viewport viewports[1] = { vk::Viewport(0, 0, static_cast<float>(width),  static_cast<float>(-height), 0, 1) }; // Inverse height as vulkan is from top left, not bottom left.
				if (m_pso.Swapchain)
				{
					viewports[0].height = height;
				}
				m_commandList.setViewport(0, 1, viewports);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::SetScissor(int width, int height)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::SetScissor] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Recording) { std::cout << "[GPUCommandList_Vulkan::SetScissor] CommandList is not recording.\n"; return; }

				vk::Rect2D scissors[1] = { vk::Rect2D({0, 0}, {static_cast<u32>(width), static_cast<u32>(height)}) };
				m_commandList.setScissor(0, 1, scissors);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::Draw] CommandList is null.\n"; return; }
				if (!CanDraw()) { std::cout << "[GPUCommandList_Vulkan::DrawIndexed] Unable to draw.\n"; return; }
				m_commandList.draw(vertexCount, instanceCount, firstVertex, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::DrawIndexed] CommandList is null.\n"; return; }
				if (!CanDraw()) { std::cout << "[GPUCommandList_Vulkan::DrawIndexed] Unable to draw.\n"; return; }
				m_commandList.drawIndexed(indexCount, instanceCount, firstIndex,vertexOffset, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::Submit] CommandList is null.\n"; return; }
				if (m_recordCommandCount == 0) { std::cout << "[GPUCommandList_Vulkan::Submit] Record command count is 0. Nothing is sunmited.\n"; return; }

				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				std::vector<vk::Semaphore> waitSemaphoresVulkan;
				std::vector<vk::Semaphore> signalSemaphoresVulkan;
				std::vector<vk::CommandBuffer> commandListVulkan = { m_commandList };
				vk::Fence fenceVulkan;

				vk::SubmitInfo submitInfo = vk::SubmitInfo(waitSemaphoresVulkan, { }, commandListVulkan, signalSemaphoresVulkan);
				GetDevice()->GetQueue(queue).submit(submitInfo, fenceVulkan);
			}

			void GPUCommandList_Vulkan::BeginRecord()
			{
				Reset();
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::BeginRecord] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Idle) { std::cout << "[GPUCommandList_Vulkan::BeginRecord] CommandList is already recording.\n"; return; }
				m_state = GPUCommandListState::Recording;

				vk::CommandBufferBeginInfo info = vk::CommandBufferBeginInfo();
				m_commandList.begin(info);
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::EndRecord()
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::EndRecord] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Recording) { std::cout << "[GPUCommandList_Vulkan::EndRecord] CommandList is not recording.\n"; return; }
				
				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				m_state = GPUCommandListState::Idle;
				m_commandList.end();
				++m_recordCommandCount;
			}

			void GPUCommandList_Vulkan::BeginRenderpass()
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::BeginRenderpass] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Recording) { std::cout << "[GPUCommandList_Vulkan::BeginRenderpass] CommandList is not recording.\n"; return; }
				if (m_activeItems.Renderpass) { std::cout << "[GPUCommandList_Vulkan::BeginRenderpass] Renderpass must call End before Begin can be called.\n"; return; }

				vk::RenderPass renderpass = GPURenderpassManager_Vulkan::Instance().GetOrCreateRenderpass({ m_pso.RenderTargets });
				vk::Rect2D rect = vk::Rect2D({}, { static_cast<u32>(Window::Instance().GetWidth()), static_cast<u32>(Window::Instance().GetHeight()) });
				std::vector<vk::ImageView> imageViews;
				std::vector<vk::ClearValue> clearColours;

				if (m_pso.Swapchain)
				{
					GPUSwapchain_Vulkan* swapchainVulkan = dynamic_cast<GPUSwapchain_Vulkan*>(GetDevice()->GetSwapchain());
					imageViews.push_back(swapchainVulkan->GetImageView());

					vk::ClearValue clearValue;
					clearValue.color.float32[0] = 0;
					clearValue.color.float32[1] = 0;
					clearValue.color.float32[2] = 0;
					clearValue.color.float32[3] = 1;
					clearColours.push_back(clearValue);
				}
				else
				{
					for (size_t i = 0; i < m_pso.RenderTargets.size(); ++i)
					{
						const RenderTarget* rt = m_pso.RenderTargets[i];
						const RenderTargetDesc rtDesc = rt->GetDesc();
						const glm::vec4 cc = rtDesc.ClearColour;
						vk::ClearValue clearValue;
						if (PixelFormatExtensions::IsDepthStencil(rtDesc.Format))
						{
							clearValue.depthStencil.depth = cc.x;
							clearValue.depthStencil.stencil = static_cast<u32>(cc.y);
							clearColours.push_back(clearValue);
						}
						else
						{
							clearValue.color.float32[0] = cc.x;
							clearValue.color.float32[1] = cc.y;
							clearValue.color.float32[2] = cc.z;
							clearValue.color.float32[3] = cc.w;
							clearColours.push_back(clearValue);
						}
						// Get image views.
					}
				}

				if (m_framebuffer)
				{
					GetDevice()->GetDevice().destroyFramebuffer(m_framebuffer);
					m_framebuffer = nullptr;
				}

				vk::FramebufferCreateInfo frameBufferInfo = vk::FramebufferCreateInfo({}, renderpass, imageViews, rect.extent.width, rect.extent.height, 1);
				m_framebuffer = GetDevice()->GetDevice().createFramebuffer(frameBufferInfo);

				vk::RenderPassBeginInfo info = vk::RenderPassBeginInfo(renderpass, m_framebuffer, rect, clearColours);
				m_commandList.beginRenderPass(info, vk::SubpassContents::eInline);
				++m_recordCommandCount;
				m_activeItems.Renderpass = true;
			}

			void GPUCommandList_Vulkan::EndRenderpass()
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::EndRenderpass] CommandList is null.\n"; return; }
				if (m_state != GPUCommandListState::Recording) { std::cout << "[GPUCommandList_Vulkan::EndRenderpass] CommandList is not recording.\n"; return; }
				if (!m_activeItems.Renderpass) { std::cout << "[GPUCommandList_Vulkan::BeginRenderpass] Renderpass must call Begin before End can be called.\n"; return; }

				m_commandList.endRenderPass();
				++m_recordCommandCount;
				m_activeItems.Renderpass = false;
			}

			void GPUCommandList_Vulkan::BindPipeline(GPUPipelineStateObject* pipeline)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::DrawIndexed] CommandList is null.\n"; return; }
				const GPUPipelineStateObject_Vulkan* pipelineVulkan = dynamic_cast<GPUPipelineStateObject_Vulkan*>(pipeline);

				// Check if we need to also begin a render pass?
				m_commandList.bindPipeline(GPUQueueToVulkanBindPoint(pipelineVulkan->GetPSO().Queue), pipelineVulkan->GetPipeline());
				++m_recordCommandCount;
			}



			GPUComamndListAllocator_Vulkan::GPUComamndListAllocator_Vulkan()
			{ }

			GPUComamndListAllocator_Vulkan::~GPUComamndListAllocator_Vulkan()
			{ 
				FreeAllCommandLists();
				Destroy();
			}

			GPUCommandList* GPUComamndListAllocator_Vulkan::AllocateCommandList(GPUCommandListType type)
			{
				vk::CommandPool& commandPool = m_commandPools[type];
				if (!commandPool)
				{
					vk::CommandPoolCreateInfo poolCreateInfo = vk::CommandPoolCreateInfo({}, GetDevice()->GetQueueFamilyIndex(m_queue));
					commandPool = GetDevice()->GetDevice().createCommandPool(poolCreateInfo);
				}

				vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
				std::vector<vk::CommandBuffer> commandBuffers = GetDevice()->GetDevice().allocateCommandBuffers(allocInfo);

				GPUCommandList_Vulkan* cmdList = new GPUCommandList_Vulkan();
				cmdList->m_commandList = commandBuffers.front();
				cmdList->m_queue = m_queue;
				cmdList->m_type = type;
				m_allocatedCommandLists.push_back(cmdList);

				return cmdList;
			}

			void GPUComamndListAllocator_Vulkan::ResetCommandLists(std::list<GPUCommandList*> cmdLists)
			{
				// TOOD:
			}

			void GPUComamndListAllocator_Vulkan::ResetCommandPool(GPUCommandListType type)
			{
				vk::CommandPool& cmdPool = m_commandPools[type];
				if (cmdPool)
				{
					GetDevice()->GetDevice().resetCommandPool(cmdPool);
				}
			}

			void GPUComamndListAllocator_Vulkan::FreeCommandList(GPUCommandList* cmdList)
			{
				FreeCommandLists({ m_allocatedCommandLists });
			}

			void GPUComamndListAllocator_Vulkan::FreeCommandLists(const std::list<GPUCommandList*>& cmdLists)
			{
				if (m_allocatedCommandLists.size() == 0)
				{
					return;
				}

				std::unordered_map<GPUCommandListType, std::vector<vk::CommandBuffer>> commndBuffersVulkan;
				for (const std::list<GPUCommandList*>::iterator::value_type& ptr : cmdLists)
				{
					std::list<GPUCommandList*>::iterator cmdListItr = std::find(m_allocatedCommandLists.begin(), m_allocatedCommandLists.end(), ptr);
					if (cmdListItr == m_allocatedCommandLists.end())
					{
						continue;
					}
					GPUCommandList_Vulkan* cmdListVulkan = dynamic_cast<GPUCommandList_Vulkan*>(*cmdListItr);

					commndBuffersVulkan[cmdListVulkan->GetType()].push_back(cmdListVulkan->GetCommandBufferVulkan());
					delete cmdListVulkan;
					m_allocatedCommandLists.erase(cmdListItr);
				}

				for (const auto& kvp : commndBuffersVulkan)
				{
					vk::CommandPool& cmdPool = m_commandPools[kvp.first];
					if (!cmdPool)
					{
						std::cout << "[GPUComamndListAllocator_Vulkan::FreeCommandLists] CommandPool is null.\n";
						continue;
					}
					GetDevice()->GetDevice().freeCommandBuffers(cmdPool, kvp.second);
				}
			}

			void GPUComamndListAllocator_Vulkan::FreeAllCommandLists()
			{
				std::list<GPUCommandList*> list = m_allocatedCommandLists;
				FreeCommandLists(list);
			}

			void GPUComamndListAllocator_Vulkan::Destroy()
			{
				for (const auto& pair : m_commandPools)
				{
					GetDevice()->GetDevice().destroyCommandPool(pair.second);
				}
				m_commandPools.clear();
			}
		}
	}
}