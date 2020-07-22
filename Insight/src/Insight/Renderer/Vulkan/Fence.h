#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Fence
		{
		public:
			Fence(const Device* device);
			~Fence();

			VkFence& GetFence() { return m_fence; }
			void SetInUse() { m_inUse = true; }
			const bool& GetInUse() const { return m_inUse; }
			void Wait() const;
			void Reset();

		private:
			const Device* m_device;
			bool m_inUse = false;
			VkFence m_fence;
		};
	}
}
#endif