#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/CommandPool.h"
#include "Insight/Renderer/Vulkan/Device.h"
#include "Insight/Renderer/Vulkan/Queue.h"
#include "Insight/Renderer/Vulkan/QueueFamily.h"
#include "Insight/Renderer/Vulkan/CommandBuffer.h"

namespace Insight
{
	namespace Render
	{
		CommandPool::CommandPool(const Device* device, const VkCommandPoolCreateFlags& createFlags)
			: m_device(device)
		{
			VkCommandPoolCreateInfo createInfo = VulkanInits::CommandPoolInfo(createFlags, device->GetQueueFamily(QueueFamilyType::Graphics).GetValue());
			ThrowIfFailed(vkCreateCommandPool(m_device->GetDevice(), &createInfo, nullptr, &m_pool));
		}

		CommandPool::~CommandPool()
		{
			vkDestroyCommandPool(m_device->GetDevice(), m_pool, nullptr);

			for (auto it = m_commandBuffers.begin(); it != m_commandBuffers.end(); ++it)
			{
				Memory::MemoryManager::DeleteOnFreeList((*it));
			}
		}

		void CommandPool::FreeCommandBuffers()
		{
			std::vector<VkCommandBuffer> buffers;
			for (auto it = m_commandBuffers.begin(); it != m_commandBuffers.end(); ++it)
			{
				buffers.push_back((*it)->GetBuffer());
			}
			vkFreeCommandBuffers(m_device->GetDevice(), m_pool, buffers.size(), buffers.data());

			for (auto it = m_commandBuffers.begin(); it != m_commandBuffers.end(); ++it)
			{
				Memory::MemoryManager::DeleteOnFreeList(*it);
			}
			m_commandBuffers.clear();
		}

		std::vector<CommandBuffer*> CommandPool::AllocCommandBuffers(const int& count)
		{
			VkCommandBufferAllocateInfo info = VulkanInits::CommandBufferAllocInfo(m_pool, count);

			std::vector<CommandBuffer*> commandBuffers;
			commandBuffers.resize(count);
			std::vector<VkCommandBuffer> vkCommandBuffers;
			vkCommandBuffers.resize(count);

			ThrowIfFailed(vkAllocateCommandBuffers(m_device->GetDevice(), &info, vkCommandBuffers.data()));

			int i = 0;
			for (auto it = commandBuffers.begin(); it != commandBuffers.end(); ++it)
			{
				(*it) = Memory::MemoryManager::NewOnFreeList<CommandBuffer>();
				(*it)->GetBuffer() = vkCommandBuffers[i++];
				m_commandBuffers.push_back(*it);
			}

			return commandBuffers;
		}

		CommandBuffer* CommandPool::AllocCommandBuffer()
		{
			VkCommandBufferAllocateInfo info = VulkanInits::CommandBufferAllocInfo(m_pool, 1);
			CommandBuffer* commandBuffer = Memory::MemoryManager::NewOnFreeList<CommandBuffer>();

			ThrowIfFailed(vkAllocateCommandBuffers(m_device->GetDevice(), &info, &commandBuffer->GetBuffer()));
			
			m_commandBuffers.push_back(commandBuffer);

			return m_commandBuffers[m_commandBuffers.size() - 1];
		}
	}
}
#endif