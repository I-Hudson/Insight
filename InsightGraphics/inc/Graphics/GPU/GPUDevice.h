#pragma once

#include "Graphics/GPU/GPUSemaphore.h"
#include "Graphics/GPU/GPUShader.h"
#include "Graphics/GPU/Enums.h"

namespace Insight
{
	namespace Graphics
	{
		class GPUAdapter;
		class GPUSwapchain;

		/*
			GPUDevice: This contains the logical and physical device for any gpu use.
		*/
		class GPUDevice
		{
		public:
			virtual ~GPUDevice() { }

			virtual bool Init() = 0;
			virtual void Destroy() = 0;

			virtual void WaitForGPU() const = 0;

			virtual GPUAdapter* GetAdapter() = 0;

			static GPUDevice* Create();

			GPUSwapchain* GetSwapchain() { return m_swapchain; }

		protected:
			GPUSwapchain* m_swapchain{ nullptr };
		};

		class GPUDeviceResource
		{ };
	}
}