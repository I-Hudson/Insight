#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RenderContext.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/DX12/RHI_Queue_DX12.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RHI/RHI_GPUCrashTracker.h"

#ifdef IS_RESOURCE_HANDLES_ENABLED
#include "Graphics/RHI/RHI_ResourcePool.h"
#endif

#include "D3D12MemAlloc.h"

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

				virtual void SetFullScreen() override;

				virtual void GpuWaitForIdle() override;
				virtual void SubmitCommandListAndWait(RHI_CommandList* cmdList) override;

				virtual void MarkTimeStamp(RHI_CommandList* cmdList) override;
				virtual std::vector<u64> ResolveTimeStamps(RHI_CommandList* cmdList) override;
				virtual u64 GetTimeStampFrequency();

				/// @brief Execute anything that is not directly graphics related like uploading data to the GPU.
				virtual void ExecuteAsyncJobs(RHI_CommandList* cmdList) override;

				void SetObjectName(std::string_view name, ID3D12Object* handle);

				virtual RHI_Texture* GetSwaphchainIamge() const override;

				ID3D12Device* GetDevice() const { return m_device.Get(); }
				DescriptorHeap_DX12& GetDescriptorHeap(DescriptorHeapTypes descriptorHeapType);
				DescriptorHeapGPU_DX12& GetFrameDescriptorHeapGPU();
				DescriptorHeapGPU_DX12& GetFrameDescriptorHeapGPUSampler();

				DescriptorHeapHandle_DX12 GetDescriptorCBVNullHandle() const;
				DescriptorHeapHandle_DX12 GetDescriptorSRVNullHandle() const;
				DescriptorHeapHandle_DX12 GetDescriptorUAVNullHandle() const;
				DescriptorHeapHandle_DX12 GetDescriptorSAMNullHandle() const;

				D3D12MA::Allocator* GetAllocator() const { return m_d3d12MA; }

#ifdef IS_RESOURCE_HANDLES_ENABLED
				virtual RHI_Handle<Texture> CreateTexture(const Texture texture) override;
				virtual void FreeTexture(const RHI_Handle<Texture> handle) override;
				virtual void UploadToTexture(const RHI_Handle<Texture> handle, const std::vector<u8>& data) override;
				virtual Texture* GetTexture(const RHI_Handle<Texture> handle) const override;
#endif

			protected:
				virtual void WaitForGpu() override;

			private:
				void FindPhysicalDevice(IDXGIAdapter1** ppAdapter);
				void ResizeSwapchainBuffers();

			private:
				RHI_PhysicalDevice_DX12 m_physicalDevice;
				ComPtr<IDXGIFactory4> m_factory{ nullptr };
				ComPtr<ID3D12Device> m_device{ nullptr };
				ComPtr<ID3D12Debug> m_debugController{ nullptr };
				D3D12MA::ALLOCATION_CALLBACKS m_d3d12maAllocationCallbacks;

#ifdef IS_RESOURCE_HANDLES_ENABLED
				RHI_ResourcePool<TextureDrawData_DX12, Texture> m_texturePool;
#endif

				ID3D12QueryHeap* m_timeStampQueryHeap = nullptr;
				u64 m_timeStampCurrentIndex = 0;
				u64 m_timeStampCurrentCount = 0;
				u64 m_timeStampPreviousCount = 0;
				bool m_timeStampEnoughFrames = false;
				u64 m_timeStampQueryMaxCountPerFrame = 64;
				RHI_Buffer* m_timeStampReadbackBuffer = nullptr;

				/// @brief D3D12 memory allocator used for all resource (buffer/textures) allocations.
				D3D12MA::Allocator* m_d3d12MA = nullptr;

				RHI_GPUCrashTracker* m_gpuCrashTracker = nullptr;

				D3D_FEATURE_LEVEL m_d3dFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_1_0_CORE;

				RHI_Queue_DX12 m_graphicsQueue;
				std::map<GPUQueue, ComPtr<ID3D12CommandQueue>> m_queues;
			
				std::array<DescriptorHeap_DX12, static_cast<u64>(DescriptorHeapTypes::NumDescriptors)> m_descriptorHeaps;
				DescriptorHeapHandle_DX12 m_cbvNullHandle;
				DescriptorHeapHandle_DX12 m_srvNullHandle;
				DescriptorHeapHandle_DX12 m_uavNullHandle;
				DescriptorHeapHandle_DX12 m_samNullHandle;

				ComPtr<IDXGISwapChain3> m_swapchain{ nullptr };
				u32 m_rtvDescriptorSize;

				std::vector<SwapchainImage> m_swapchainImages;

				int m_currentFrame = 0;
				u32 m_availableSwapchainImage = 0;

				std::thread m_rendererThread;
				std::atomic<bool> m_rendererThreadShutdown = false;
				FrameResource<FrameSubmitContext_DX12> m_submitFrameContexts;

				FrameResource<u64> m_submitFenceValues;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)