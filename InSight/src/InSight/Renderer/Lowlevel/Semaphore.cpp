#include "ispch.h"
#include "Semaphore.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/VulkanInits.h"

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