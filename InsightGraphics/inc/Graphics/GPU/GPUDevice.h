#pragma once

#include "Graphics/GraphicsManager.h"
#include "Graphics/GPU/GPUSemaphore.h"

namespace Insight
{
	namespace Graphics
	{
		class GPUAdapter;

		enum GPUQueue
		{
			GPUQueue_Graphics,
			GPUQueue_Compute,
			GPUQueue_Transfer,
		};

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

			GPUSemaphoreManager& GetSemaphoreManager() { return m_semaphoreManager; }

		protected:
			GPUSemaphoreManager m_semaphoreManager;
		};

		class GPUDeviceResource
		{ };
	}
}