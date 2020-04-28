#include "ispch.h"
#include "Queue.h"

#include "Insight/Renderer/Lowlevel/Device.h"

namespace Insight
{
	namespace Render
	{
		Queue::Queue(const Device* device, const QueueFamily& queueFamily)
			: m_device(device), m_queueFamily(queueFamily)
		{
		}

		Queue::~Queue()
		{
		}

		const QueueFamilyType Queue::GetType() const
		{
			return m_queueFamily.GetType();
		}

		VkQueue& Queue::GetQueue()
		{
			return m_queue;
		}

		void Queue::Submit(QueueInfo& queueInfo)
		{
			switch (m_queueFamily.GetType())
			{
			case QueueFamilyType::Graphics:
					GraphicsQueueInfo graphicsInfo = static_cast<GraphicsQueueInfo&>(queueInfo);
					graphicsInfo.SyncFence->SetInUse();
					ThrowIfFailed(vkQueueSubmit(m_queue, 1, graphicsInfo.SubmitInfo, graphicsInfo.SyncFence->GetFence()));
					break;

			case QueueFamilyType::Present:
				PresentQueueInfo presentInfo = static_cast<PresentQueueInfo&>(queueInfo);
				ThrowIfFailed(vkQueuePresentKHR(m_queue, presentInfo.PresentInfo));
				break;
			}
		}

		void Queue::Wait() const
		{
			switch (m_queueFamily.GetType())
			{
			case QueueFamilyType::Graphics:
				break;

			case QueueFamilyType::Present:
				ThrowIfFailed(vkQueueWaitIdle(m_queue));
				break;
			}
		}

		VkSemaphore Queue::SubmitSem()
		{
			return VkSemaphore();
		}
	}
}

