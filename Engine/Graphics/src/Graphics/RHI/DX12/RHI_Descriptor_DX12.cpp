#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include "Graphics/PixelFormatExtensions.h"

#include "Core/Logger.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			u32 DescriptorHeap_DX12::s_currentHeapId = 0;

			DescriptorHeapHandle_DX12::DescriptorHeapHandle_DX12()
				: CPUPtr({ 0 })
				, GPUPtr({ 0 })
				, HeapId(-1)
				, HeapType(DescriptorHeapTypes::CBV_SRV_UAV)
			{ }

			DescriptorHeapHandle_DX12::DescriptorHeapHandle_DX12(u64 cpuPtr, u64 gpuPtr, u32 headId, DescriptorHeapTypes heapType)
				: CPUPtr({ cpuPtr })
				, GPUPtr({ gpuPtr })
				, HeapId(headId)
				, HeapType(heapType)
			{ }

			//// <summary>
			//// DescriptorHeapPage_DX12
			//// </summary>
			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12()
			{ }

			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12(int capacity, DescriptorHeapTypes type, RenderContext_DX12* context, u32 heapId, bool gpuVisable)
			{
				m_heapType = type;
				m_capacity = static_cast<u32>(capacity);
				m_context = context;
				m_heapId = heapId;
				m_gpuVisable = gpuVisable;

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = DescriptorHeapTypeToDX12(m_heapType);
				heapDesc.Flags = m_gpuVisable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				heapDesc.NumDescriptors = static_cast<UINT>(m_capacity);

				m_context->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
				std::wstring heapName = Platform::WStringFromString(DescriptorHeapTypeToString(m_heapType)) + L"_" + std::to_wstring(heapId) + L"_GPUVisable_" + (gpuVisable ? L"True" : L"False");
				m_heap->SetName(heapName.c_str());

				m_descriptorSize = m_context->GetDevice()->GetDescriptorHandleIncrementSize(heapDesc.Type);

				m_descriptorHeapCPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				if (m_gpuVisable)
				{
					m_descriptorHeapGPUStart = m_heap->GetGPUDescriptorHandleForHeapStart();
				}

				for (size_t i = 0; i < m_capacity; ++i)
				{
					DescriptorHeapHandle_DX12 freeHandle(m_descriptorHeapCPUStart.ptr + (i * m_descriptorSize), 
						m_gpuVisable ? m_descriptorHeapGPUStart.ptr + (i * m_descriptorSize) : 0ull,
						m_heapId, 
						m_heapType);
					m_freeHandles.push_back(freeHandle);
				}
			}

			bool DescriptorHeapPage_DX12::GetNewHandle(DescriptorHeapHandle_DX12& handle)
			{
				ASSERT(m_heap);

				if (m_freeHandles.empty())
				{
					return false;
				}

				handle = m_freeHandles.back();
				m_freeHandles.pop_back();

				return true;
			}

			void DescriptorHeapPage_DX12::FreeHandle(DescriptorHeapHandle_DX12& handle)
			{
				ASSERT(m_heap);
				if (handle.HeapId == m_heapId)
				{
					m_freeHandles.push_back(handle);
					handle = {};
				}
			}

			void DescriptorHeapPage_DX12::Reset()
			{
				m_freeHandles.clear();
				for (size_t i = 0; i < m_capacity; ++i)
				{
					DescriptorHeapHandle_DX12 freeHandle(m_descriptorHeapCPUStart.ptr + (i * m_descriptorSize),
						m_gpuVisable ? m_descriptorHeapGPUStart.ptr + (i * m_descriptorSize) : 0ull,
						m_heapId,
						m_heapType);
					m_freeHandles.push_back(freeHandle);
				}
			}

			void DescriptorHeapPage_DX12::Destroy()
			{
				m_freeHandles.clear();
				if (m_heap)
				{
					m_heap->Release();
					m_heap = nullptr;
				}
			}


			//// <summary>
			//// DescriptorHeap_DX12
			//// </summary>
			//// <param name="heapType"></param>
			void DescriptorHeap_DX12::Create(DescriptorHeapTypes heapType)
			{
				Create(heapType, 256);
			}

			void DescriptorHeap_DX12::Create(DescriptorHeapTypes heapType, u32 handleCount)
			{
				m_heapType = heapType;
				AddNewHeap(handleCount);
			}

			ID3D12DescriptorHeap* DescriptorHeap_DX12::GetHeap(u32 heapId) const
			{
				return m_heaps.at(heapId).GetHeap();
			}

			DescriptorHeapHandle_DX12 DescriptorHeap_DX12::GetNewHandle()
			{
				DescriptorHeapHandle_DX12 handle = { };
				// Check for a handle from all heaps before creating a new one.
				for (auto& heap : m_heaps)
				{
					if (heap.GetNewHandle(handle))
					{
						return handle;
					}
				}

				// All heaps are full. Create a new heap and get a handle from.
				AddNewHeap();
				m_heaps.back().GetNewHandle(handle);

				return handle;
			}

			void DescriptorHeap_DX12::FreeHandle(DescriptorHeapHandle_DX12& handle)
			{
				for (auto heap : m_heaps)
				{
					heap.FreeHandle(handle);
				}
			}

			void DescriptorHeap_DX12::Reset()
			{
				m_descriptorHeapHandle.clear();
				m_descriptorHashToHandleIndex.clear();
				for (auto& heap : m_heaps)
				{
					heap.Reset();
				}
			}

			void DescriptorHeap_DX12::Destroy()
			{
				Reset();
				for (auto& heap : m_heaps)
				{
					heap.Destroy();
				}
			}

			void DescriptorHeap_DX12::AddNewHeap()
			{
				AddNewHeap(256);
			}

			void DescriptorHeap_DX12::AddNewHeap(u32 handleCount)
			{
				DescriptorHeapPage_DX12 heap(handleCount, m_heapType, m_context, s_currentHeapId++, m_isGPUVisalbe);
				m_heaps.push_back(heap);
			}

			//---------------------------------------------
			// DescriptorSubHeapGPU_DX12
			//---------------------------------------------
#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
			DescriptorSubHeapPageGPU_DX12::DescriptorSubHeapPageGPU_DX12(u64 cpuStartPointer, u64 gpuStartPointer, u64 descriptorSize, u64 capacity)
				: m_cpuStartPointer(cpuStartPointer)
				, m_gpuStartPointer(gpuStartPointer)
				, m_descriptorSize(descriptorSize)
				, m_capcaity(capacity)
				, m_size(0)
			{ }

			void DescriptorSubHeapGPU_DX12::AddPage(DescriptorSubHeapPageGPU_DX12 page)
			{
			}

			DescriptorSubHeapGPU_DX12 DescriptorHeapGPU_DX12::AllocateSubHeap()
			{
				DescriptorSubHeapGPU_DX12 heap = {};
				GrowSubHeap(heap);
				return heap;
			}
#endif

			//---------------------------------------------
			// DescriptorHeapGPU_DX12
			//---------------------------------------------
			DescriptorHeapGPU_DX12::DescriptorHeapGPU_DX12()
			{
			}

			DescriptorHeapGPU_DX12::~DescriptorHeapGPU_DX12()
			{
				Destroy();
			}

			void DescriptorHeapGPU_DX12::Create(DescriptorHeapTypes heapType, u32 handleCount, std::string name)
			{
				Destroy();
				m_heapType = heapType;
				m_capacity = static_cast<u32>(handleCount);

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = DescriptorHeapTypeToDX12(m_heapType);
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				heapDesc.NumDescriptors = static_cast<UINT>(m_capacity);

				ThrowIfFailed(m_context->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
				m_descriptorSize = m_context->GetDevice()->GetDescriptorHandleIncrementSize(heapDesc.Type);
				m_context->SetObjectName(name, m_heap);

				m_descriptorHeapCPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				m_descriptorHeapGPUStart = m_heap->GetGPUDescriptorHandleForHeapStart();
			}

			DescriptorHeapHandle_DX12 DescriptorHeapGPU_DX12::GetNextHandle()
			{
				ASSERT(m_currentDescriptorIndex < m_capacity);

				DescriptorHeapHandle_DX12 handle(
					m_descriptorHeapCPUStart.ptr + (m_currentDescriptorIndex * m_descriptorSize),
					m_descriptorHeapGPUStart.ptr + (m_currentDescriptorIndex * m_descriptorSize),
					0,
					m_heapType);
				++m_currentDescriptorIndex;
				return handle;
			}

			ID3D12DescriptorHeap* DescriptorHeapGPU_DX12::GetHeap() const
			{
				ASSERT(m_heap);
				return m_heap;
			}

#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
			void DescriptorHeapGPU_DX12::GrowSubHeap(DescriptorSubHeapGPU_DX12& subHeap)
			{
				const u64 capacity = 2024;
				//std::unique_lock lock(m_subAllocMutex);
				DescriptorSubHeapPageGPU_DX12 page(m_descriptorHeapCPUStart.ptr + m_subHeapCPUOffset, m_descriptorHeapGPUStart.ptr + m_subHeapGPUOffset, m_descriptorSize, capacity);
				m_subHeapCPUOffset += (capacity * m_descriptorSize);
				m_subHeapGPUOffset += (capacity * m_descriptorSize);
				//lock.unlock();
				subHeap.AddPage(page);

			}
#endif // IS_DESCRIPTOR_MULTITHREAD_DX12

			void DescriptorHeapGPU_DX12::Reset()
			{
				m_currentDescriptorIndex = 0;
#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
				//std::unique_lock lock(m_subAllocMutex);
				m_subHeapCPUOffset = 0;
				m_subHeapGPUOffset = 0; 
				//lock.unlock();
#endif // IS_DESCRIPTOR_MULTITHREAD_DX12
			}

			void DescriptorHeapGPU_DX12::Destroy()
			{
				if (m_heap)
				{
					m_heap->Release();
					m_heap = nullptr;
				}
			}
}
	}
}

#endif /// if defined(IS_DX12_ENABLED)