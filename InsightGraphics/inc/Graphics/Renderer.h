#pragma once

#include "Graphics/GPU/GPUCommandList.h"

namespace Insight
{
	namespace Graphics
	{
		class GPUDevice;
		class GPUSwapchain;
		class GPUSemaphore;
		class GPUFence;

		constexpr const char* CMD_RENDERER = "CMD_RENDERER";

		class Renderer
		{
		public:

			void Init(GPUDevice* gpuDevice);
			void Destroy();

			void Render();

		private:
			// List all passes.
			void Prepare(GPUCommandList* cmdList);
			void Submit(GPUCommandList* cmdList);

		private:
			GPUDevice* m_gpuDevice{ nullptr };

			struct Frame
			{
				GPUSemaphore* PresentCompleteSemaphore{ nullptr };
				GPUFence* Fence{ nullptr };

				void Init();
				void Destroy();
			};
			std::vector<Frame> m_frames;
			int m_imageIndex;
		};
	}
}