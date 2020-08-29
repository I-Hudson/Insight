#include "ispch.h"
#ifdef IS_VULKAN
#include "CommandBuffer.h"
#include "VulkanInits.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Platform
{
	CommandBuffer::CommandBuffer()
	{
	}

	CommandBuffer::~CommandBuffer()
	{
	}

	void CommandBuffer::StartRecord(const VkCommandBufferUsageFlags& usageFlags,
		const VkCommandBufferInheritanceInfo* inheritanceInfo)
	{
		IS_PROFILE_FUNCTION();
		VkCommandBufferBeginInfo beginInfo = VulkanInits::CommandBufferBeginInfo(usageFlags, inheritanceInfo);

		ThrowIfFailed(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
	}

	void CommandBuffer::EndRecord()
	{
		IS_PROFILE_FUNCTION();
		ThrowIfFailed(vkEndCommandBuffer(m_commandBuffer));
	}
}
#endif