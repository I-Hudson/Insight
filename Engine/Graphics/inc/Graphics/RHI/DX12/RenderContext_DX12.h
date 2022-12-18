#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			struct SwapchainImage
			{
				ComPtr<ID3D12Resource> Colour;
				DescriptorHeapHandle_DX12 ColourHandle;
				ComPtr<ID3D12Resource> DepthStencil;
				DescriptorHeapHandle_DX12 DepthStencilHandle;
			};

			struct FrameSubmitContext_DX12
			{
				ComPtr<ID3D12Fence> SubmitFence{ nullptr };
				u64 SubmitFenceValue;
				HANDLE SubmitFenceEvent;
				SwapchainImage Images;
				std::vector<RHI_CommandList*> CommandLists;
			};

			class RenderContext_DX12 : public RenderContext
			{
			public:

				virtual bool Init() override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual bool PrepareRender() override;
				virtual void PreRender(RHI_CommandList* cmdList) override;
				virtual void PostRender(RHI_CommandList* cmdList) override;

				virtual void SetSwaphchainResolution(glm::ivec2 resolution) override;
				virtual glm::ivec2 GetSwaphchainResolution() const override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				void SetObjectName(std::string_view name, ID3D12Object* handle);

				virtual RHI_Texture* GetSwaphchainIamge() const override { return nullptr; }

				ID3D12Device* GetDevice() const { return m_device.Get(); }

			protected:
				virtual void WaitForGpu() override;

			private:
				void FindPhysicalDevice(IDXGIAdapter1** ppAdapter);
				void CreateSwapchain(u32 width, u32 height);
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

				DescriptorHeap_DX12 m_rtvHeap;
				DescriptorHeap_DX12 m_dsvHeap;
				ComPtr<ID3D12DescriptorHeap> m_srcImGuiHeap{ nullptr };

				int m_currentFrame = 0;

				FrameResource<FrameSubmitContext_DX12> m_submitFrameContexts;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)