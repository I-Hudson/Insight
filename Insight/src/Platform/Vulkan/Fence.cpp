#include "ispch.h"
#ifdef IS_VULKAN
#include "Fence.h"
#include "Device.h"
#include "Insight/Instrumentor/Instrumentor.h"

namespace Platform
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
		IS_PROFILE_FUNCTION();

		if (m_inUse)
		{
			vkWaitForFences(m_device->GetDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
		}
	}

	void Fence::Reset()
	{
		IS_PROFILE_FUNCTION();

		vkResetFences(m_device->GetDevice(), 1, &m_fence);
		m_inUse = false;
	}
}
#endif