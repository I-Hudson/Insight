#include "ispch.h"
#ifdef IS_VULKAN

#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/CommandBuffer.h"

namespace Insight
{
	namespace Render
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
			VkCommandBufferBeginInfo beginInfo = VulkanInits::CommandBufferBeginInfo(usageFlags, inheritanceInfo);

			ThrowIfFailed(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
		}

		void CommandBuffer::EndRecord()
		{
			ThrowIfFailed(vkEndCommandBuffer(m_commandBuffer));
		}
	}
}
#endif