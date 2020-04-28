#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

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
