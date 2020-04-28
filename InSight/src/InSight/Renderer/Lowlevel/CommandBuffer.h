#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

namespace Insight
{
	namespace Render
	{
		class CommandBuffer
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
}