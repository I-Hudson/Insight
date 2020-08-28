#include "ispch.h"
#ifdef IS_VULKAN
#include "Semaphore.h"
#include "Device.h"

namespace Platform
{
	Semaphore::Semaphore(const Device* device)
		: m_device(device)
	{
		ThrowIfFailed(vkCreateSemaphore(m_device->GetDevice(), &VulkanInits::SemaphoreInfo(), nullptr, &m_semaphore));
	}

	Semaphore::~Semaphore()
	{
		vkDestroySemaphore(m_device->GetDevice(), m_semaphore, nullptr);
	}
}
#endif