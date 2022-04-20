#pragma once

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/CommandList_DX12.h"
#include "Graphics/RHI/DX12/PipelineStateObjectManager_DX12.h"
#include "Core/Logger.h"
#include <glm/glm.hpp>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12 : public RenderContext
			{
			public:

				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual void Render(CommandList cmdList) override;

				ID3D12Device* GetDevice() const { return m_device.Get(); }
				PipelineStateObjectManager_DX12& GetPipelineStateObjectManager() { return m_pipelineStateObjectManager; }

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
				ComPtr<ID3D12DescriptorHeap> m_rtvHeap{ nullptr };
				u32 m_rtvDescriptorSize;
				std::vector<ComPtr<ID3D12Resource>> m_swapchainImages;
				glm::ivec2 m_swapchainSize;

				PipelineStateObjectManager_DX12 m_pipelineStateObjectManager;

				HANDLE m_fenceEvent;
				ComPtr<ID3D12Fence> m_swapchainFence{ nullptr };
				u64 m_swapchainFenceValues[c_FrameCount];

				ComPtr<ID3D12DescriptorHeap> m_srcImGuiHeap{ nullptr };


				int m_currentFrame = 0;
				int m_frameIndex = 0;

				struct FrameResourceDX12
				{
					CommandAllocator_DX12 CommandAllocator;
					RenderContext_DX12* Context;

					void Init(RenderContext_DX12* context);
					void Destroy();
				};
				FrameResourceDX12 m_frames[c_FrameCount];
			};
		}
	}
}