#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"

#include "Insight/Renderer/Vulkan/Queue.h"
#include "Insight/Renderer/Vulkan/Device.h"

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

		QueueFamily& Queue::GetQueueFamily()
		{
			return m_queueFamily;
		}

		void Queue::Submit(QueueInfo& queueInfo)
		{
			VkResult result{};

			switch (m_queueFamily.GetType())
			{
			case QueueFamilyType::Graphics:
					GraphicsQueueInfo graphicsInfo = static_cast<GraphicsQueueInfo&>(queueInfo);
					if (graphicsInfo.SyncFence != nullptr || graphicsInfo.SyncFence != VK_NULL_HANDLE)
					{
						graphicsInfo.SyncFence->SetInUse();
						ThrowIfFailed(result = vkQueueSubmit(m_queue, 1, graphicsInfo.SubmitInfo, graphicsInfo.SyncFence->GetFence()));
					}
					else
					{
						ThrowIfFailed(result = vkQueueSubmit(m_queue, 1, graphicsInfo.SubmitInfo,VK_NULL_HANDLE));
					}

					if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
					{
						IS_CORE_INFO("Swapchain is out of date");
					}
					break;

			case QueueFamilyType::Present:
				PresentQueueInfo presentInfo = static_cast<PresentQueueInfo&>(queueInfo);
				ThrowIfFailed(vkQueuePresentKHR(m_queue, presentInfo.PresentInfo));
				if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
				{
					IS_CORE_INFO("Swapchain is out of date");
				}
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
#endif