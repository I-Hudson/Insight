#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Semaphore
		{
		public:
			Semaphore(const Device* device);
			~Semaphore();

			VkSemaphore& GetSemaphore() { return m_semaphore; }

		private:
			const Device* m_device;
			VkSemaphore m_semaphore;
		};
	}
}
#endif