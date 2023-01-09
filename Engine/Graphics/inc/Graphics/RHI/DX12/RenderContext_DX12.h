#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include <array>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Texture_DX12;
			
			struct SwapchainImage
			{
				RHI_Texture_DX12* Colour;
				DescriptorHeapHandle_DX12 ColourHandle;
			};

			struct FrameSubmitContext_DX12
			{
				ComPtr<ID3D12Fence> SubmitFence{ nullptr };
				u64 SubmitFenceValue;
				HANDLE SubmitFenceEvent;
				DescriptorHeapGPU_DX12 DescriptorHeapGPURes;
				DescriptorHeapGPU_DX12 DescriptorHeapSampler;
				std::vector<RHI_CommandList*> CommandLists;

				void OnCompleted();
			};

			class RenderContext_DX12 : public RenderContext
			{
			public:
				virtual ~RenderContext_DX12();

				virtual bool Init(RenderContextDesc desc) override;
				virtual void Destroy() override;

				virtual void InitImGui() override;
				virtual void DestroyImGui() override;

				virtual bool PrepareRender() override;
				virtual void PreRender(RHI_CommandList* cmdList) override;
				virtual void PostRender(RHI_CommandList* cmdList) override;

				virtual void CreateSwapchain(SwapchainDesc desc) override;
				virtual void SetSwaphchainResolution(glm::ivec2 resolution) override;
				virtual glm::ivec2 GetSwaphchainResolution() const override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				void SetObjectName(std::string_view name, ID3D12Object* handle);

				virtual RHI_Texture* GetSwaphchainIamge() const override;

				ID3D12Device* GetDevice() const { return m_device.Get(); }
				DescriptorHeap_DX12& GetDescriptorHeap(DescriptorHeapTypes descriptorHeapType);
				DescriptorHeapGPU_DX12& GetFrameDescriptorHeapGPU();
				DescriptorHeapGPU_DX12& GetFrameDescriptorHeapGPUSampler();

			protected:
				virtual void WaitForGpu() override;

			private:
				void FindPhysicalDevice(IDXGIAdapter1** ppAdapter);
				void ResizeSwapchainBuffers();

				void WaitForNextFrame();

			private:
				RHI_PhysicalDevice_DX12 m_physicalDevice;
				ComPtr<IDXGIFactory4> m_factory{ nullptr };
				ComPtr<ID3D12Device> m_device{ nullptr };
				ComPtr<ID3D12Debug> m_debugController{ nullptr };

				std::map<GPUQueue, ComPtr<ID3D12CommandQueue>> m_queues;
			
				std::array<DescriptorHeap_DX12, static_cast<u64>(DescriptorHeapTypes::NumDescriptors)> m_descriptorHeaps;

				ComPtr<IDXGISwapChain3> m_swapchain{ nullptr };
				u32 m_rtvDescriptorSize;

				std::vector<SwapchainImage> m_swapchainImages;

				int m_currentFrame = 0;
				u32 m_availableSwapchainImage = 0;

				FrameResource<FrameSubmitContext_DX12> m_submitFrameContexts;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)