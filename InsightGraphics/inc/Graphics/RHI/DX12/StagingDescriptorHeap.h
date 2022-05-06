#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include <vector>
#include <unordered_map>

namespace std
{
	using DescriptorType = Insight::Graphics::DescriptorType;
	template<>
	struct hash<DescriptorType>
	{
		_NODISCARD size_t operator()(const DescriptorType& val) const 
		{
			const size_t i = static_cast<const size_t>(val);
			return std::hash<size_t>()(i);
		}
	};
}

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class DescriptorHeap
			{
			public:
				DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors, bool isReferencedByShader);
				virtual ~DescriptorHeap();

				ID3D12DescriptorHeap* GetHeap() { return mDescriptorHeap; }
				D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return mHeapType; }
				D3D12_CPU_DESCRIPTOR_HANDLE GetHeapCPUStart() { return mDescriptorHeapCPUStart; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetHeapGPUStart() { return mDescriptorHeapGPUStart; }
				u32 GetMaxDescriptors() { return mMaxDescriptors; }
				u32 GetDescriptorSize() { return mDescriptorSize; }

			protected:
				ID3D12DescriptorHeap* mDescriptorHeap;
				D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
				D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHeapCPUStart;
				D3D12_GPU_DESCRIPTOR_HANDLE mDescriptorHeapGPUStart;
				u32 mMaxDescriptors;
				u32 mDescriptorSize;
				bool   mIsReferencedByShader;
			};

			class DescriptorHeapHandle
			{
			public:
				DescriptorHeapHandle()
				{
					mCPUHandle.ptr = NULL;
					mGPUHandle.ptr = NULL;
					mHeapIndex = 0;
				}

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return mCPUHandle; }
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return mGPUHandle; }
				u32 GetHeapIndex() { return mHeapIndex; }

				void SetCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { mCPUHandle = cpuHandle; }
				void SetGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { mGPUHandle = gpuHandle; }
				void SetHeapIndex(u32 heapIndex) { mHeapIndex = heapIndex; }

				bool IsValid() { return mCPUHandle.ptr != NULL; }
				bool IsReferencedByShader() { return mGPUHandle.ptr != NULL; }

			private:
				D3D12_CPU_DESCRIPTOR_HANDLE mCPUHandle;
				D3D12_GPU_DESCRIPTOR_HANDLE mGPUHandle;
				u32 mHeapIndex;
			};

			class StagingDescriptorHeap : public DescriptorHeap
			{
			public:
				StagingDescriptorHeap();
				StagingDescriptorHeap(ID3D12Device * device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 numDescriptors);
				~StagingDescriptorHeap();


				DescriptorHeapHandle GetNewHeapHandle();
				void FreeHeapHandle(DescriptorHeapHandle handle);

			private:
				std::vector<u32> mFreeDescriptors;
				u32 mCurrentDescriptorIndex;
				u32 mActiveHandleCount;
			};

			class StagingDescriptorHeapManager
			{
			public:
				~StagingDescriptorHeapManager();

				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }

				DescriptorHeapHandle GetNewHeapHandle(DescriptorType type);
				void FreeHeapHandle(DescriptorType type, DescriptorHeapHandle handle);

			private:
				RenderContext_DX12* m_context;
				std::unordered_map<DescriptorType, StagingDescriptorHeap> m_heaps;
			};
		}
	}
}