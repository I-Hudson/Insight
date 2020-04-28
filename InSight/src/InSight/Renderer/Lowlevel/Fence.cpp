#include "ispch.h"
#include "Fence.h"

#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Renderer/VulkanInits.h"

namespace Insight
{
	namespace Render
	{
		Fence::Fence(const Device* device)
			: m_device(device)
		{
			ThrowIfFailed(vkCreateFence(m_device->GetDevice(), &VulkanInits::FenceInfo(), nullptr, &m_fence));
		}

		Fence::~Fence()
		{
			vkDestroyFence(m_device->GetDevice(), m_fence, nullptr);
		}

		void Fence::Wait() const
		{
			vkWaitForFences(m_device->GetDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
		}

		void Fence::Reset() const
		{
			vkResetFences(m_device->GetDevice(), 1, &m_fence);

		}
	}
}