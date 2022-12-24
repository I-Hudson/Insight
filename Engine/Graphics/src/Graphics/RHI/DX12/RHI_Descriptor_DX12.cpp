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
			//// <summary>
			//// RHI_DescriptorLayout_DX12
			//// </summary>
			//// <param name="context"></param>
			//// <param name="set"></param>
			//// <param name="descriptors"></param>
			//void RHI_DescriptorLayout_DX12::Create(RenderContext* context, int set, DescriptorSet descriptor_set)
			//{
			//	m_context = static_cast<RenderContext_DX12*>(context);
			//
			//	/// Reference: https:///github.com/shuhuai/DeferredShadingD3D12/blob/master/DeferredRender.cpp
			//
			//	///std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
			//	///rootParameters.resize(descriptors.size());
			//	///CD3DX12_ROOT_PARAMETER1 rootParameter = {};
			//	///rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
			//	///rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
			//	///rootParameters.push_back(rootParameter);
			//
			//	///int baseRegister = 0;
			//	///for (const Descriptor& descriptor : descriptors)
			//	///{
			//	///	CD3DX12_DESCRIPTOR_RANGE1 range = {};
			//	///	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, baseRegister++);
			//	///	ranges.push_back(range);
			//	///
			//	///	CD3DX12_ROOT_PARAMETER1 rootParameter = {};
			//	///	rootParameter.InitAsDescriptorTable(1, &ranges.back());
			//	///	rootParameters.push_back(rootParameter);
			//	///}
			//
			//	std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
			//	for (const DescriptorBinding& desc : descriptor_set.Bindings)
			//	{
			//		CD3DX12_DESCRIPTOR_RANGE1 range = {};
			//		range.Init(DescriptorRangeTypeToDX12(desc.Type), 1, desc.Binding);
			//		ranges.push_back(range);
			//	}
			//
			//	int rangeIndex = 0;
			//	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
			//	rootParameters.resize(ranges.size());
			//	for (CD3DX12_ROOT_PARAMETER1& root : rootParameters)
			//	{
			//		root.InitAsDescriptorTable(1, &ranges[rangeIndex++]);
			//	}
			//
			//	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
			//	rootSignatureDesc.Init_1_1((UINT)rootParameters.size(), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
			//	
			//	/// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
			//	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			//	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			//
			//	if (FAILED(m_context->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			//	{
			//		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			//	}
			//
			//	ComPtr<ID3DBlob> signature;
			//	ComPtr<ID3DBlob> error;
			//	if(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error) != S_OK)
			//	{
			//		IS_CORE_ERROR("[RHI_DescriptorLayout_DX12::Create] Error: {}", error->GetBufferPointer());
			//	}
			//	ThrowIfFailed(m_context->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_layout)));
			//}

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
				heapDesc.Flags = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV  && m_gpuVisable) ?
					D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
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

			void DescriptorHeapPage_DX12::FreeHandle(DescriptorHeapHandle_DX12 handle)
			{
				ASSERT(m_heap);
				if (handle.HeapId == m_heapId)
				{
					m_freeHandles.push_back(handle);
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
				m_heapType = heapType;
				AddNewHeap();
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
				DescriptorHeapPage_DX12 heap(256, m_heapType, m_context, s_currentHeapId++, m_isGPUVisalbe);
				m_heaps.push_back(heap);
			}

			//---------------------------------------------
			// DescriptorHeapGPU_DX12
			//---------------------------------------------
			DescriptorHeapGPU_DX12::DescriptorHeapGPU_DX12()
			{
				m_isGPUVisalbe = true;
			}

			DescriptorHeapGPU_DX12::~DescriptorHeapGPU_DX12()
			{
				Destroy();
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)