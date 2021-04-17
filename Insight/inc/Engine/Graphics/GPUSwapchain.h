#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Module/WindowModule.h"

namespace Insight::Graphics
{
	class GPUSemaphore;
	class GPUImage;
	class GPUImageView;

	struct GPUSwapchainDesc
	{
		GPUSwapchainDesc()
			: Width(Window::GetWidth()), Height(Window::GetHeight())
			, VSync(false), GSync(true)
		{ }
		GPUSwapchainDesc(u32 width, u32 height, bool vsync, bool gsync)
			: Width(width), Height(height)
			, VSync(vsync), GSync(gsync)
		{ }

		u32 Width;
		u32 Height;
		bool VSync;
		bool GSync;
	};

	struct SwapchainImage
	{
		GPUImage* Image;
		GPUImageView* View;
	};

	class GPUSwapchain
	{
	public:
		GPUSwapchain() { }
		virtual ~GPUSwapchain() { }

		static GPUSwapchain* New();

		virtual void Init() = 0;
		virtual void Build(const GPUSwapchainDesc& desc) = 0;

		virtual GPUResults GetNextImage(GPUSemaphore* presentCompleted, u32* imageIndex) = 0;
		virtual GPUResults Present(GPUQueue queue, u32 imageIndex, std::vector<GPUSemaphore*> waitSemaphores = {}) = 0;
		virtual void ReleaseGPU() = 0;
		virtual u32 GetImageCount() = 0;
		GPUImage* GetImage(u32 imageIndex) { return m_swapchainImages[imageIndex].Image; }

	protected:
		GPUSwapchainDesc m_desc;
		std::vector<SwapchainImage> m_swapchainImages;
		u32 m_swapchainImageCount;
	};
}