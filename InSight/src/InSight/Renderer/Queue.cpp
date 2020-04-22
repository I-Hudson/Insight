#include "ispch.h"
#include "Queue.h"

#include "Insight/Renderer/Device.h"

namespace Insight
{
	namespace Render
	{
		Queue::Queue(const Device device, const QueueFamily queueFamily)
		{
		}

		Queue::~Queue()
		{
		}

		const QueueFamilyType Queue::GetType() const
		{
			return m_queueFamily.GetType();
		}

		const VkQueue& Queue::GetQueue()
		{
			return m_queue;
		}

		void Queue::Wait() const
		{
		}

		VkSemaphore Queue::SubmitSem()
		{
			return VkSemaphore();
		}
	}
}

