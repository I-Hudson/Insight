#pragma once

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

			class RHI_DescriptorLayout_DX12 : public RHI_DescriptorLayout
			{
			public:
				ID3D12RootSignature* GetRootSignature() { return m_layout.Get(); }

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			protected:
				virtual void Create(RenderContext* context, int set, std::vector<Descriptor> descriptors) override;

			private:
				RenderContext_DX12* m_context{ nullptr };
				ComPtr<ID3D12RootSignature> m_layout{ nullptr };
			};

			class RHI_Descriptor_DX12 : public RHI_Descriptor
			{
			public:

				virtual void Update(const std::vector<Descriptor>& descriptors) override;
				virtual u64 GetHash(bool includeResouce = false) override;

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			private:
				RenderContext_DX12* m_context{ nullptr };
			};

			struct DescriptorHeapHandle_DX12
			{
				D3D12_CPU_DESCRIPTOR_HANDLE CPUPtr{ 0 };
				D3D12_GPU_DESCRIPTOR_HANDLE GPUPtr{ 0 };
				int HandleIndex = -1;

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUPtr; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return GPUPtr; }
				u32 GetHeapIndex() { return HandleIndex; }

				void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { CPUPtr = cpuHandle; }
				void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { GPUPtr = gpuHandle; }
				void SetHeapIndex(u32 heapIndex) { HandleIndex = heapIndex; }

				bool IsValid() const { return CPUPtr.ptr != NULL; }
				bool IsReferencedByShader() const { return GPUPtr.ptr != NULL; }
			};

			class DescriptorHeapPage_DX12
			{
			public:
				DescriptorHeapPage_DX12();
				DescriptorHeapPage_DX12(int capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, RenderContext_DX12* context);

				ID3D12DescriptorHeap* GetHeap() const { return m_heap.Get(); }
				D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_heapType; }
				u32 GetDescriptorSize() const { return m_descriptorSize; }
				u64 GetSize() const { return m_size; }
				u64 GetCapacity() const { return m_capacity; }
				bool IsFull() const { return GetSize() == GetCapacity(); }
				bool GetNewHandle(DescriptorHeapHandle_DX12& handle);
				void FreeHandle(DescriptorHeapHandle_DX12 handle);
				void Reset();

			private:
				ComPtr<ID3D12DescriptorHeap> m_heap;
				u64 m_size = 0;
				u64 m_capacity = 0;
				u32 m_descriptorSize = 0;
				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

				std::vector<int> m_freeSlots;
				std::vector<int> m_allocateIndexs;
				D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapCPUStart;
				D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHeapGPUStart;

				friend DescriptorHeap_DX12;
			};

			class DescriptorHeap_DX12
			{
			public:

				void Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }

				const std::vector<DescriptorHeapPage_DX12>& GetHeaps() const { return m_heaps; }

				DescriptorHeapHandle_DX12 GetNewHandle();
				bool FindDescriptor(const Descriptor& descriptor, DescriptorHeapHandle_DX12& handle);

				void Reset();

			private:
				void AddNewHeap();

			private:
				RenderContext_DX12* m_context = nullptr;
				D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;
				std::vector<DescriptorHeapPage_DX12> m_heaps;

				std::unordered_map<int, DescriptorHeapHandle_DX12> m_descriptorHeapHandle;
				std::unordered_map<u64, int> m_descriptorHashToHandleIndex; // Keep track of all the slots for the descriptors.
			};

			class DescriptorAllocator_DX12 : public DescriptorAllocator
			{
			public:

				bool SetupDescriptors();
				std::vector<ID3D12DescriptorHeap*> GetHeaps() const;
				void SetDescriptors(CommandList_DX12* cmdList);

				void SetDescriptorTables();
				void BindDescriptorTables(ID3D12GraphicsCommandList* cmdList);
				void BindTempConstentBuffer(ID3D12GraphicsCommandList* cmdList, RHI_BufferView bufferView, u32 rootParameterIndex);

				// DescriptorAllocator
				virtual void SetRenderContext(RenderContext* context) override;
				virtual bool GetDescriptors(std::vector<RHI_Descriptor*>& descriptors) override;
				virtual void Reset() override;
				virtual void Destroy() override;

			private:
				RenderContext_DX12* m_context = nullptr;
				std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, DescriptorHeap_DX12> m_heaps;
				PipelineStateObject m_pso;

				std::vector<std::pair<int, DescriptorHeapHandle_DX12>> m_descrptorTables;
				std::unordered_map<int, std::unordered_map<int, u64>> m_boundDescriptors;
			};
		}
	}
}