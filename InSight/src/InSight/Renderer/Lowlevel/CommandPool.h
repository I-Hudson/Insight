#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"

#include <vector>

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API CommandPool
		{
		public:
			CommandPool(const Device* device, const VkCommandPoolCreateFlags& createFlags = 0);
			~CommandPool();

			VkCommandPool& GetCommandPool() { return m_pool; }
			void FreeCommandBuffers();

			std::vector<CommandBuffer*> AllocCommandBuffers(const int& count);
			CommandBuffer* AllocCommandBuffer();

		private:
			const Device* m_device;
			VkCommandPool m_pool;

			std::vector<CommandBuffer*> m_commandBuffers;
		};
	}
}