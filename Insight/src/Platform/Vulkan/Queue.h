#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"
#include "QueueFamily.h"
#include "Fence.h"

namespace Platform
{
	class Device;

	struct QueueInfo
	{ };

	struct GraphicsQueueInfo : public QueueInfo
	{
		VkSubmitInfo* SubmitInfo;
		Fence* SyncFence;
	};

	struct PresentQueueInfo : public QueueInfo
	{
		VkPresentInfoKHR* PresentInfo;
	};

	class IS_API Queue
	{
	public:
		Queue(const Device* device, const QueueFamily& queueFamily);
		~Queue();

		const QueueFamilyType GetType() const;
		VkQueue& GetQueue();
		QueueFamily& GetQueueFamily();

		void Submit(QueueInfo& queueInfo);
		void Wait() const;

		VkSemaphore SubmitSem();

	private:
		const Device* m_device;
		QueueFamily m_queueFamily;
		VkQueue m_queue;
	};
}
#endif