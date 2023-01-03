#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include "Graphics/PixelFormatExtensions.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			u32 DescriptorHeap_DX12::s_currentHeapId = 0;

			//// <summary>
			//// DescriptorHeapPage_DX12
			//// </summary>
			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12()
			{ }

			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12(int capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, RenderContext_DX12* context, u32 heapId, bool gpuVisable)
			{
				m_heapType = type;
				m_capacity = static_cast<u32>(capacity);
				m_heapId = heapId;
				m_gpuVisable = gpuVisable;

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = m_heapType;
				heapDesc.Flags = m_gpuVisable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				heapDesc.NumDescriptors = static_cast<UINT>(m_capacity);

				context->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
				m_descriptorSize = context->GetDevice()->GetDescriptorHandleIncrementSize(m_heapType);

				m_descriptorHeapCPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				if (m_gpuVisable)
				{
					m_descriptorHeapGPUStart = m_heap->GetGPUDescriptorHandleForHeapStart();
				}

				for (size_t i = 0; i < m_capacity; ++i)
				{
					DescriptorHeapHandle_DX12 freeHandle = {};
					freeHandle.CPUPtr.ptr = m_descriptorHeapCPUStart.ptr + (i * m_descriptorSize);
					if (m_gpuVisable)
					{
						freeHandle.GPUPtr.ptr = m_descriptorHeapGPUStart.ptr + (i * m_descriptorSize);
					}
					freeHandle.HeapId = m_heapId;
					m_freeHandles.push_back(freeHandle);
				}
			}

			bool DescriptorHeapPage_DX12::GetNewHandle(DescriptorHeapHandle_DX12& handle)
			{
				ASSERT(m_heap);

				if (m_freeHandles.empty())
				{
					IS_CORE_ERROR("[DescriptorHeapPage_DX12::GetNewHandle] Heap is full.");
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
					DescriptorHeapHandle_DX12 freeHandle = {};
					freeHandle.CPUPtr.ptr = m_descriptorHeapCPUStart.ptr + (i * m_descriptorSize);
					if (m_gpuVisable)
					{
						freeHandle.GPUPtr.ptr = m_descriptorHeapGPUStart.ptr + (i * m_descriptorSize);
					}
					freeHandle.HeapId = m_heapId;
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
			void DescriptorHeap_DX12::Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
			{
				Create(heapType, 256);
			}

			void DescriptorHeap_DX12::Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 handleCount)
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
			// DescriptorHeapGPU_DX12
			//---------------------------------------------
			DescriptorHeapGPU_DX12::DescriptorHeapGPU_DX12()
			{
			}

			DescriptorHeapGPU_DX12::~DescriptorHeapGPU_DX12()
			{
				Destroy();
			}

			void DescriptorHeapGPU_DX12::Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 handleCount)
			{
				Destroy();
				m_heapType = heapType;
				m_capacity = static_cast<u32>(handleCount);

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = m_heapType;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				heapDesc.NumDescriptors = static_cast<UINT>(m_capacity);

				ThrowIfFailed(m_context->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
				m_descriptorSize = m_context->GetDevice()->GetDescriptorHandleIncrementSize(m_heapType);

				m_descriptorHeapCPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				m_descriptorHeapGPUStart = m_heap->GetGPUDescriptorHandleForHeapStart();
			}

			DescriptorHeapHandle_DX12 DescriptorHeapGPU_DX12::GetNextHandle()
			{
				ASSERT(m_currentDescriptorIndex < m_capacity);

				DescriptorHeapHandle_DX12 handle;
				handle.CPUPtr.ptr = m_descriptorHeapCPUStart.ptr + (m_currentDescriptorIndex * m_descriptorSize);
				handle.GPUPtr.ptr = m_descriptorHeapGPUStart.ptr + (m_currentDescriptorIndex * m_descriptorSize);
				++m_currentDescriptorIndex;
				return handle;
			}

			ID3D12DescriptorHeap* DescriptorHeapGPU_DX12::GetHeap() const
			{
				ASSERT(m_heap);
				return m_heap;
			}

			void DescriptorHeapGPU_DX12::Reset()
			{
				m_currentDescriptorIndex = 0;
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