#include "Graphics/GPU/GPUSwapchain.h"

#include "Graphics/GPU/RHI/Vulkan/GPUSwapchain_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		GPUSwapchain* GPUSwapchain::Create()
		{
			return new RHI::Vulkan::GPUSwapchain_Vulkan();
		}

		void GPUSwapchain::AcquireNextImage(GPUSemaphore* semaphore)
		{
			AcquireNextImage(semaphore, nullptr);
		}

		void GPUSwapchain::Present(GPUQueue queue, const std::vector<GPUSemaphore*>& semaphores)
		{
			Present(queue, m_nextImgeIndex, semaphores);
		}
	}
}