#include "ispch.h"
#ifdef IS_VULKAN
#include "Queue.h"
#include "Device.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Platform
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

	QueueFamily& Queue::GetQueueFamily()
	{
		return m_queueFamily;
	}

	VkResult Queue::Submit(const VkSubmitInfo& queueInfo, VkFence fence)
	{
		IS_PROFILE_FUNCTION();

		VkResult result;
		if (fence != VK_NULL_HANDLE)
		{
			//queueInfo.SyncFence->SetInUse();
			ThrowIfFailed(result = vkQueueSubmit(m_queue, 1, &queueInfo, fence));
		}
		else
		{
			ThrowIfFailed(result = vkQueueSubmit(m_queue, 1, &queueInfo, VK_NULL_HANDLE));
		}
		return result;
	}

	VkResult Queue::Presnet(const VkPresentInfoKHR& presentInfo)
	{
		VkResult result;
		ThrowIfFailed(result = vkQueuePresentKHR(m_queue, &presentInfo));
		return result;
	}

	void Queue::Wait() const
	{
		IS_PROFILE_FUNCTION();

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
#endif