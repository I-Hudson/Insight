#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include <unordered_set>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;
			class CommandList_DX12;
			class DescriptorHeap_DX12;
			class RHI_CommandList_DX12;

			enum DescriptorHeapTypes
			{
				CBV_SRV_UAV,
				Sampler,
				RenderTargetView,
				DepthStencilView,

				NumDescriptors
			};

			struct DescriptorHeapHandle_DX12
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUPtr{ 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUPtr{ 0 };
				u32 HeapId = 0;

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUPtr; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return GPUPtr; }

				void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { CPUPtr = cpuHandle; }
				void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { GPUPtr = gpuHandle; }

				bool IsValid() const { return CPUPtr.ptr != NULL; }
				bool IsReferencedByShader() const { return GPUPtr.ptr != NULL; }
			};

			class DescriptorHeapPage_DX12
			{
			public:
				DescriptorHeapPage_DX12();
				DescriptorHeapPage_DX12(int capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, RenderContext_DX12* context, u32 heapId, bool gpuVisable);

				ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }
				D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_heapType; }
				u32 GetDescriptorSize() const { return m_descriptorSize; }
				bool GetNewHandle(DescriptorHeapHandle_DX12& handle);
				void FreeHandle(DescriptorHeapHandle_DX12 handle);
				void Reset();

				void Destroy();

			private:
				ComPtr<ID3D12DescriptorHeap> m_heap;
				std::vector<DescriptorHeapHandle_DX12> m_freeHandles;

				u32 m_capacity = 0;
				u32 m_descriptorSize = 0;
				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

				D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCPUStart;
				D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHeapGPUStart;

				u32 m_heapId = 0;
				bool m_gpuVisable = false;
				friend DescriptorHeap_DX12;
			};

			class DescriptorHeap_DX12
			{
			public:
				virtual ~DescriptorHeap_DX12() = default;

				void Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }

				const std::vector<DescriptorHeapPage_DX12>& GetHeaps() const { return m_heaps; }
				ID3D12DescriptorHeap* GetHeap(u32 heapId) const;

				DescriptorHeapHandle_DX12 GetNewHandle();
				void FreeHandle(DescriptorHeapHandle_DX12& handle);

				void Reset();
				void Destroy();

			private:
				void AddNewHeap();

			protected:
				bool m_isGPUVisalbe = false;
			
			private:
				RenderContext_DX12* m_context = nullptr;
				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
				std::vector<DescriptorHeapPage_DX12> m_heaps;

				std::unordered_map<int, DescriptorHeapHandle_DX12> m_descriptorHeapHandle;
				std::unordered_map<u64, int> m_descriptorHashToHandleIndex; /// Keep track of all the slots for the descriptors.

				static u32 s_currentHeapId;
			};

			class DescriptorHeapGPU_DX12 : public DescriptorHeap_DX12
			{
			public:
				DescriptorHeapGPU_DX12();
				virtual ~DescriptorHeapGPU_DX12() override;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)