#include "Graphics/GPU/GPUSwapchain.h"

namespace Insight
{
	namespace Graphics
	{
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