#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/PipelineStateObjectManager_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			struct FrameResource_DX12 : public FrameResouce
			{
				RenderContext_DX12* Context;
				//DescriptorAllocator_DX12 DescriptorAllocator;

				void Init(RenderContext_DX12* context);
				void Destroy();
				virtual void Reset() override;
			};

			struct SwapchainImage
			{
				ComPtr<ID3D12Resource> Colour;
				DescriptorHeapHandle_DX12 ColourHandle;
				ComPtr<ID3D12Resource> DepthStencil;
				DescriptorHeapHandle_DX12 DepthStencilHandle;
			};

			class RenderContext_DX12 : public RenderContext
			{
			public:

				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual bool PrepareRender() override { return true; }
				virtual void PostRender(RHI_CommandList* cmdList) override { }

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				ID3D12Device* GetDevice() const { return m_device.Get(); }
				PipelineStateObjectManager_DX12& GetPipelineStateObjectManager() { return m_pipelineStateObjectManager; }

				FrameResource_DX12& GetFrameResouce() { return m_frames[m_frameIndex]; }

			protected:
				virtual void WaitForGpu() override;

			private:
				void FindPhysicalDevice(IDXGIAdapter1** ppAdapter);
				void CreateSwapchain();
				void ResizeSwapchainBuffers();

				void WaitForNextFrame();

			private:
				RHI_PhysicalDevice_DX12 m_physicalDevice;
				ComPtr<IDXGIFactory4> m_factory{ nullptr };
				ComPtr<ID3D12Device> m_device{ nullptr };
				ComPtr<ID3D12Debug> m_debugController{ nullptr };

				std::map<GPUQueue, ComPtr<ID3D12CommandQueue>> m_queues;
			
				ComPtr<IDXGISwapChain3> m_swapchain{ nullptr };
				u32 m_rtvDescriptorSize;

				std::vector<SwapchainImage> m_swapchainImages;
				glm::ivec2 m_swapchainSize;

				PipelineStateObjectManager_DX12 m_pipelineStateObjectManager;

				HANDLE m_fenceEvent;
				ComPtr<ID3D12Fence> m_swapchainFence{ nullptr };
				std::vector<u64> m_swapchainFenceValues;

				DescriptorHeap_DX12 m_rtvHeap;
				DescriptorHeap_DX12 m_dsvHeap;
				ComPtr<ID3D12DescriptorHeap> m_srcImGuiHeap{ nullptr };

				int m_currentFrame = 0;
				int m_frameIndex = 0;

				std::vector<FrameResource_DX12> m_frames;
			};
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)