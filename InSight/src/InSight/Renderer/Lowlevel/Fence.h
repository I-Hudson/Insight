#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Vulkan.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Fence
		{
		public:
			Fence(const Device* device);
			~Fence();

			VkFence& GetFence() { return m_fence; }
			void SetInUse() { m_inUse = true; }
			const bool& GetInUse() const { return m_inUse; }
			void Wait() const;
			void Reset() const;

		private:
			const Device* m_device;
			bool m_inUse = false;
			VkFence m_fence;
		};
	}
}