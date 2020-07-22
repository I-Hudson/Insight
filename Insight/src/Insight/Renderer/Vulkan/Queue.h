#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Insight/Renderer/Vulkan/QueueFamily.h"
#include "Insight/Renderer/Vulkan/Fence.h"

namespace Insight
{
	namespace Render
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

			void Submit(QueueInfo& queueInfo);
			void Wait() const;

			VkSemaphore SubmitSem();

		private:
			const Device* m_device;
			QueueFamily m_queueFamily;
			VkQueue m_queue;
		};
	}
}
#endif