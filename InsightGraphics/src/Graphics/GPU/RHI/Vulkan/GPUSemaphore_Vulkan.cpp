#include "Graphics/GPU/RHI/Vulkan/GPUSemaphore_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			void GPUSemaphore_Vulkan::Signal()
			{
				vk::SemaphoreSignalInfo info = vk::SemaphoreSignalInfo(m_semaphore, 1Ui64);
				GetDevice()->GetDevice().signalSemaphore(info);
			}

			void GPUSemaphore_Vulkan::Create(bool signaled)
			{
				if (!m_semaphore)
				{
					vk::SemaphoreCreateInfo createInfo = vk::SemaphoreCreateInfo();
					m_semaphore = GetDevice()->GetDevice().createSemaphore(createInfo);
				}
			}

			void GPUSemaphore_Vulkan::Destroy()
			{
				if (m_semaphore)
				{
					GetDevice()->GetDevice().destroySemaphore(m_semaphore);
					m_semaphore = vk::Semaphore(nullptr);
				}
			}
		}
	}
}