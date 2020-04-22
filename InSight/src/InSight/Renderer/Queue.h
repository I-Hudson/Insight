#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/QueueFamily.h"

#include "Vulkan.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Queue
		{
		public:
			Queue(const Device device, const QueueFamily queueFamily);
			~Queue();

			const QueueFamilyType GetType() const;
			const VkQueue& GetQueue();

			void Wait() const;

			VkSemaphore SubmitSem();

		private:
			QueueFamily m_queueFamily;
			VkQueue m_queue;
		};
	}
}


