#include "ispch.h"
#ifdef IS_VULKAN
#include "Vulkan.h"
#include "Insight/Memory/MemoryManager.h"
#include "CommandPool.h"
#include "Device.h"
#include "Queue.h"
#include "QueueFamily.h"
#include "CommandBuffer.h"

namespace Platform
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
			DELETE_ON_HEAP((*it));
		}
	}

	void CommandPool::FreeCommandBuffers()
	{
		std::vector<VkCommandBuffer> buffers;
		for (auto it = m_commandBuffers.begin(); it != m_commandBuffers.end(); ++it)
		{
			buffers.push_back((*it)->GetBuffer());
		}
		vkFreeCommandBuffers(m_device->GetDevice(), m_pool, (uint32_t)buffers.size(), buffers.data());

		for (auto it = m_commandBuffers.begin(); it != m_commandBuffers.end(); ++it)
		{
			DELETE_ON_HEAP(*it);
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
			(*it) = NEW_ON_HEAP(CommandBuffer);
			(*it)->GetBuffer() = vkCommandBuffers[i++];
			m_commandBuffers.push_back(*it);
		}

		return commandBuffers;
	}

	CommandBuffer* CommandPool::AllocCommandBuffer()
	{
		VkCommandBufferAllocateInfo info = VulkanInits::CommandBufferAllocInfo(m_pool, 1);
		CommandBuffer* commandBuffer = NEW_ON_HEAP(CommandBuffer);

		ThrowIfFailed(vkAllocateCommandBuffers(m_device->GetDevice(), &info, &commandBuffer->GetBuffer()));

		m_commandBuffers.push_back(commandBuffer);

		return m_commandBuffers[m_commandBuffers.size() - 1];
	}
}
#endif