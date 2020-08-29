#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"

namespace Platform
{
	class IS_API CommandBuffer
	{
	public:
		CommandBuffer();
		~CommandBuffer();

		VkCommandBuffer& GetBuffer() { return m_commandBuffer; }

		void StartRecord(const VkCommandBufferUsageFlags& usageFlags = 0,
			const VkCommandBufferInheritanceInfo* inheritanceInfo = nullptr);
		void EndRecord();

	private:
		VkCommandBuffer m_commandBuffer;
	};
}
#endif