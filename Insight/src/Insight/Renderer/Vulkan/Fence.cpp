#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/Fence.h"
#include "Insight/Renderer/Vulkan/Device.h"

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
			if (m_inUse)
			{
				vkWaitForFences(m_device->GetDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
			}
		}

		void Fence::Reset()
		{
			vkResetFences(m_device->GetDevice(), 1, &m_fence);
			m_inUse = false;
		}
	}
}
#endif