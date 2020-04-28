#include "ispch.h"
#include "CommandBuffer.h"

#include "Insight/Renderer/VulkanInits.h"

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