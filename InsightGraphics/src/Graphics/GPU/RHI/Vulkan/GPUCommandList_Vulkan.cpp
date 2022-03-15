#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"
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
			{ }

			void GPUCommandList_Vulkan::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::Draw] CommandList is null.\n"; return; }
				m_commandList.draw(vertexCount, instanceCount, firstVertex, firstInstance);
			}

			void GPUCommandList_Vulkan::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::DrawIndexed] CommandList is null.\n"; return; }
				m_commandList.drawIndexed(indexCount, instanceCount, firstIndex,vertexOffset, firstInstance);
			}

			void GPUCommandList_Vulkan::Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::Submit] CommandList is null.\n"; return; }

				std::vector<vk::Semaphore> waitSemaphoresVulkan;
				std::vector<vk::Semaphore> signalSemaphoresVulkan;
				std::vector<vk::CommandBuffer> commandListVulkan = { m_commandList };
				vk::Fence fenceVulkan;

				vk::SubmitInfo submitInfo = vk::SubmitInfo(waitSemaphoresVulkan, { }, commandListVulkan, signalSemaphoresVulkan);
				GetDevice()->GetQueue(queue).submit(submitInfo, fenceVulkan);
			}

			void GPUCommandList_Vulkan::SubmitAndWait(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence)
			{
				if (!m_commandList) { std::cout << "[GPUCommandList_Vulkan::SubmitAndWait] CommandList is null.\n"; return; }
				// Get fence,
				// Wait on fence,
				Submit(queue, waitSemaphores, signalSemaphores, fence);
				// Return fence to manager.
			}



			GPUComamndListAllocator_Vulkan::GPUComamndListAllocator_Vulkan()
			{ }

			GPUComamndListAllocator_Vulkan::~GPUComamndListAllocator_Vulkan()
			{ 
				FreeAllCommandLists();
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
		}
	}
}