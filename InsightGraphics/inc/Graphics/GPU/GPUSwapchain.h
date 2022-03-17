#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/GPU/GPUDevice.h"
#include "Graphics/PixelFormat.h"
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class GPUSemaphore;
		class GPUFence;

		enum GPUQueue;

		struct GPUSwapchainDesc
		{
			GPUSwapchainDesc()
				: Width(-1), Height(-1)
				, VSync(false), GSync(true)
			{ }
			GPUSwapchainDesc(int width, int height)
				: Width(width), Height(height)
				, VSync(false), GSync(true)
			{ }
			GPUSwapchainDesc(int width, int height, int imageCount, bool vsync, bool gsync)
				: Width(width), Height(height)
				, ImageCount(imageCount)
				, VSync(vsync), GSync(gsync)
			{ }

			int Width = -1;
			int Height = -1;
			int ImageCount = 0;
			bool VSync = false;
			bool GSync = true;
		};

		class GPUSwapchain
		{
		public:
			virtual ~GPUSwapchain() { }

			GPUSwapchainDesc GetDesc() const { return m_desc; }

			virtual void Prepare() = 0;
			virtual void Build(GPUSwapchainDesc desc) = 0;
			virtual void Destroy() = 0;
			
			PixelFormat GetColourFormat() const { return m_surfaceFormat; }

			void AcquireNextImage(GPUSemaphore* semaphore);
			virtual void AcquireNextImage(GPUSemaphore* semaphore, GPUFence* fence) = 0;

			void Present(GPUQueue queue, const std::vector<GPUSemaphore*>& semaphores);
			virtual void Present(GPUQueue queue, u32 imageIndex, const std::vector<GPUSemaphore*>& semaphores) = 0;

			u32 GetNextImageIndex() const { return m_nextImgeIndex; }

		protected:
			GPUSwapchainDesc m_desc;
			u32 m_nextImgeIndex;
			int m_imageCount;

			PixelFormat m_surfaceFormat;
		};
	}
}