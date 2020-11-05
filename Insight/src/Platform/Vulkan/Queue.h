#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"
#include "QueueFamily.h"
#include "Fence.h"

namespace Platform
{
	class Device;


	class IS_API Queue
	{
	public:
		Queue(const Device* device, const QueueFamily& queueFamily);
		~Queue();

		const QueueFamilyType GetType() const;
		VkQueue& GetQueue();
		QueueFamily& GetQueueFamily();

		VkResult Submit(const VkSubmitInfo& queueInfo, VkFence fence);
		VkResult Presnet(const VkPresentInfoKHR& presentInfo);
		void Wait() const;

		VkSemaphore SubmitSem();

	private:
		const Device* m_device;
		QueueFamily m_queueFamily;
		VkQueue m_queue;
	};
}
#endif