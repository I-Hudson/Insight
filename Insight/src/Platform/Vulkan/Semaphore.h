#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"

namespace Platform
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
#endif