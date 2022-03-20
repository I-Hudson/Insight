#pragma once

#include "Graphics/GPU/GPUSwapchain.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class GPUSwapchain_DX12 : public GPUSwapchain, public GPUResource_DX12
			{
			public:
				virtual ~GPUSwapchain_DX12() override;

				virtual void Prepare() override;
				virtual void Build(GPUSwapchainDesc desc) override;
				virtual void Destroy() override;

				virtual void AcquireNextImage(GPUSemaphore* semaphore, GPUFence* fence) override;
				virtual void Present(GPUQueue queue, u32 imageIndex, const std::vector<GPUSemaphore*>& semaphores) override;

			private:
				ComPtr<IDXGISwapChain3> m_swapChain;
				ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
				u32 m_rtvDescriptorSize;
				std::vector<ComPtr<ID3D12Resource>> m_swapchainImages;
			};
		}
	}
}