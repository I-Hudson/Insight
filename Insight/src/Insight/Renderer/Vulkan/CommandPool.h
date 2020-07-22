#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

namespace Insight
{
	namespace Render
	{
		class Device;
		class CommandBuffer;

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
#endif