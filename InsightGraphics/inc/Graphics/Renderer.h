#pragma once

namespace Insight
{
	namespace Graphics
	{
		class GPUDevice;
		class GPUSwapchain;
		class GPUSemaphore;

		class Renderer
		{
		public:

			void Init(GPUDevice* gpuDevice);
			void Destroy();

			void Render();

		private:
			// List all passes.
			void Prepare();
			void Submit();

		private:
			GPUDevice* m_gpuDevice{ nullptr };
			GPUSwapchain* m_swapchain{ nullptr };
			GPUSemaphore* m_presentCompleteSemaphore{ nullptr };
		};
	}
}