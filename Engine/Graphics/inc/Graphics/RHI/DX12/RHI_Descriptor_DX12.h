#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

#include <unordered_set>
#include <mutex>

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
			constexpr const char* DescriptorHeapTypeToString(DescriptorHeapTypes heapType)
			{
				switch (heapType)
				{
				case Insight::Graphics::RHI::DX12::CBV_SRV_UAV:      return "CBV_SRV_UAV";
				case Insight::Graphics::RHI::DX12::Sampler:          return "Sampler";
				case Insight::Graphics::RHI::DX12::RenderTargetView: return "RenderTargetView";
				case Insight::Graphics::RHI::DX12::DepthStencilView: return "DepthStencilView";
				case Insight::Graphics::RHI::DX12::NumDescriptors:
					break;
				default:
					break;
				}
				FAIL_ASSERT();
				return "";
			}

			enum class RootSignatureDescriptorTypes
			{
				RootConstant,
				RootDescriptor,
				DescriptorTable
			};

			struct DescriptorHeapHandle_DX12
			{
				DescriptorHeapHandle_DX12();
				DescriptorHeapHandle_DX12(u64 cpuPtr, u64 gpuPtr, u32 headId, DescriptorHeapTypes heapType);

				D3D12_CPU_DESCRIPTOR_HANDLE CPUPtr{ 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUPtr{ 0 };
				u32 HeapId = 0;
				DescriptorHeapTypes HeapType;

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUPtr; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return GPUPtr; }

				bool IsValid() const { return CPUPtr.ptr != NULL; }
				bool IsReferencedByShader() const { return GPUPtr.ptr != NULL; }
			};

			struct DescriptorCopiedHandleDX12
			{
				DescriptorHeapHandle_DX12 Handle;
				RootSignatureDescriptorTypes Type;
			};

			class DescriptorHeapPage_DX12
			{
			public:
				DescriptorHeapPage_DX12();
				DescriptorHeapPage_DX12(int capacity, DescriptorHeapTypes type, RenderContext_DX12* context, u32 heapId, bool gpuVisable);

				ID3D12DescriptorHeap* GetHeap() const { return m_heap; }
				DescriptorHeapTypes GetHeapType() const { return m_heapType; }
				u32 GetDescriptorSize() const { return m_descriptorSize; }
				bool GetNewHandle(DescriptorHeapHandle_DX12& handle);
				void FreeHandle(DescriptorHeapHandle_DX12& handle);
				void Reset();

				void Destroy();

			private:
				RenderContext_DX12* m_context = nullptr;
				ID3D12DescriptorHeap* m_heap;
				std::vector<DescriptorHeapHandle_DX12> m_freeHandles;

				u32 m_capacity = 0;
				u32 m_descriptorSize = 0;
				DescriptorHeapTypes m_heapType;

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

				void Create(DescriptorHeapTypes heapType);
				void Create(DescriptorHeapTypes heapType, u32 handleCount);
				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }

				const std::vector<DescriptorHeapPage_DX12>& GetHeaps() const { return m_heaps; }
				ID3D12DescriptorHeap* GetHeap(u32 heapId) const;

				DescriptorHeapHandle_DX12 GetNewHandle();
				void FreeHandle(DescriptorHeapHandle_DX12& handle);

				void Reset();
				void Destroy();

			private:
				void AddNewHeap();
				void AddNewHeap(u32 handleCount);

			protected:
				bool m_isGPUVisalbe = false;
			
			private:
				RenderContext_DX12* m_context = nullptr;
				DescriptorHeapTypes m_heapType;
				std::vector<DescriptorHeapPage_DX12> m_heaps;

				std::unordered_map<int, DescriptorHeapHandle_DX12> m_descriptorHeapHandle;
				std::unordered_map<u64, int> m_descriptorHashToHandleIndex; /// Keep track of all the slots for the descriptors.

				static u32 s_currentHeapId;
			};

#define IS_DESCRIPTOR_MULTITHREAD_DX12
#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12

			/// @brief 
			class DescriptorSubHeapPageGPU_DX12
			{
			public:
				DescriptorSubHeapPageGPU_DX12() = default;
				DescriptorSubHeapPageGPU_DX12(u64 cpuStartPointer, u64 gpuStartPointer, u64 descriptorSize, u64 capacity);

				u64 GetNextCPUHandle() const { return m_cpuStartPointer + (m_size * m_descriptorSize); }
				u64 GetNextGPUHandle() const { return m_gpuStartPointer + (m_size * m_descriptorSize); }

				bool IsFull() const { return m_size == m_capcaity; }

			private:
				u64 m_cpuStartPointer = 0;
				u64 m_gpuStartPointer = 0;
				u64 m_descriptorSize = 0;
				/// @brief Current number of descriptor used.
				u64 m_size = 0;
				/// @brief Total amount of descriptor available.
				u64 m_capcaity = 0;
			};

			/// @brief Manage a small subset of descriptor heaps from GPU memory.
			// This object is designed to be transient, to be destroyed and created each frame.
			class DescriptorSubHeapGPU_DX12
			{
			public:
				DescriptorSubHeapGPU_DX12() = default;

				bool IsActive() const { return m_pages.size() > 0; }

			private:
				void AddPage(DescriptorSubHeapPageGPU_DX12 page);

			private:
				std::vector<DescriptorSubHeapPageGPU_DX12> m_pages;

				friend class DescriptorHeapGPU_DX12;
			};
#endif
			/// @brief GPU visible descriptor heap.
			class DescriptorHeapGPU_DX12
			{
			public:
				DescriptorHeapGPU_DX12();
				~DescriptorHeapGPU_DX12();

				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }
				void Create(DescriptorHeapTypes heapType, u32 handleCount);

				DescriptorHeapHandle_DX12 GetNextHandle();

				ID3D12DescriptorHeap* GetHeap() const;

#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
				DescriptorSubHeapGPU_DX12 AllocateSubHeap();
				void GrowSubHeap(DescriptorSubHeapGPU_DX12& subHeap);
#endif // IS_DESCRIPTOR_MULTITHREAD_DX12

				void Reset();
				void Destroy();

			private:
				RenderContext_DX12* m_context = nullptr;

				DescriptorHeapTypes m_heapType;
				ID3D12DescriptorHeap* m_heap = nullptr;
				u32 m_currentDescriptorIndex = 0;

				u32 m_capacity = 0;
				u32 m_descriptorSize = 0;

				D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCPUStart{ 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHeapGPUStart{ 0 };

#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
				std::mutex m_subAllocMutex;
				u64 m_subHeapCPUOffset = 0;
				u64 m_subHeapGPUOffset = 0;
#endif // IS_DESCRIPTOR_MULTITHREAD_DX12
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)