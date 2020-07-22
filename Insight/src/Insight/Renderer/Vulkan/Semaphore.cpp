#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/Semaphore.h"
#include "Insight/Renderer/Vulkan/Device.h"

namespace Insight
{
	namespace Render
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
}
#endif