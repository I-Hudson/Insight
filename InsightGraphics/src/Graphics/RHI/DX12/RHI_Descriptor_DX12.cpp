#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include "Graphics/PixelFormatExtensions.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			/// <summary>
			/// RHI_DescriptorLayout_DX12
			/// </summary>
			void RHI_DescriptorLayout_DX12::Release()
			{
				if (m_layout)
				{
					m_layout.Reset();
				}
			}

			bool RHI_DescriptorLayout_DX12::ValidResouce()
			{
				return m_layout;
			}

			void RHI_DescriptorLayout_DX12::SetName(std::wstring name)
			{
				if (m_layout)
				{
					m_layout->SetName(name.c_str());
				}
			}


			///// <summary>
			///// RHI_Descriptor_DX12
			///// </summary>
			///// <param name="descriptors"></param>
			//void RHI_Descriptor_DX12::Update(const std::vector<Descriptor>& descriptors)
			//{
			//}

			//u64 RHI_Descriptor_DX12::GetHash(bool includeResouce)
			//{
			//	return 0;
			//}


			/// <summary>
			/// RHI_DescriptorLayout_DX12
			/// </summary>
			/// <param name="context"></param>
			/// <param name="set"></param>
			/// <param name="descriptors"></param>
			void RHI_DescriptorLayout_DX12::Create(RenderContext* context, int set, std::vector<Descriptor> descriptors)
			{
				m_context = static_cast<RenderContext_DX12*>(context);

				// Reference: https://github.com/shuhuai/DeferredShadingD3D12/blob/master/DeferredRender.cpp

				//std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
				//rootParameters.resize(descriptors.size());
				//CD3DX12_ROOT_PARAMETER1 rootParameter = {};
				//rootParameters[0].InitAsDescriptorTable(1, &ranges[0]);
				//rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
				//rootParameters.push_back(rootParameter);

				//int baseRegister = 0;
				//for (const Descriptor& descriptor : descriptors)
				//{
				//	CD3DX12_DESCRIPTOR_RANGE1 range = {};
				//	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, baseRegister++);
				//	ranges.push_back(range);
				//
				//	CD3DX12_ROOT_PARAMETER1 rootParameter = {};
				//	rootParameter.InitAsDescriptorTable(1, &ranges.back());
				//	rootParameters.push_back(rootParameter);
				//}

				std::vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;
				for (const Descriptor& desc : descriptors)
				{
					CD3DX12_DESCRIPTOR_RANGE1 range = {};
					range.Init(DescriptorRangeTypeToDX12(desc.Type), 1, desc.Binding);
					ranges.push_back(range);
				}

				int rangeIndex = 0;
				std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
				rootParameters.resize(ranges.size());
				for (CD3DX12_ROOT_PARAMETER1& root : rootParameters)
				{
					root.InitAsDescriptorTable(1, &ranges[rangeIndex++]);
				}

				CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
				rootSignatureDesc.Init_1_1((UINT)rootParameters.size(), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
				
				// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
				D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

				if (FAILED(m_context->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
				{
					featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
				}

				ComPtr<ID3DBlob> signature;
				ComPtr<ID3DBlob> error;
				if(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error) != S_OK)
				{
					IS_CORE_ERROR("[RHI_DescriptorLayout_DX12::Create] Error: {}", error->GetBufferPointer());
				}
				ThrowIfFailed(m_context->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_layout)));
			}

			/// <summary>
			/// DescriptorHeapPage_DX12
			/// </summary>
			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12()
			{
			}

			DescriptorHeapPage_DX12::DescriptorHeapPage_DX12(int capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, RenderContext_DX12* context)
			{
				m_capacity = capacity;
				m_heapType = type;

				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = m_heapType;
				heapDesc.Flags = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				heapDesc.NumDescriptors = (UINT)m_capacity;

				context->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
				m_descriptorSize = context->GetDevice()->GetDescriptorHandleIncrementSize(m_heapType);

				m_descriptorHeapCPUStart = m_heap->GetCPUDescriptorHandleForHeapStart();
				m_descriptorHeapGPUStart = m_heap->GetGPUDescriptorHandleForHeapStart();
			}

			bool DescriptorHeapPage_DX12::GetNewHandle(DescriptorHeapHandle_DX12& handle)
			{
				int handleIndex = 0;
				handle = { };

				if (GetSize() >= GetCapacity())
				{
					IS_CORE_ERROR("[DescriptorHeapPage_DX12::GetNewHandle] Heap is full.");
					return false;
				}
				else if (!m_freeSlots.empty())
				{
					handleIndex = m_freeSlots.back();
					m_freeSlots.pop_back();
				}
				else if (!m_allocateIndexs.empty())
				{
					handleIndex = m_allocateIndexs.back() + 1;
				}


				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_descriptorHeapCPUStart;
				cpuHandle.ptr += handleIndex * m_descriptorSize;
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_descriptorHeapGPUStart;
				gpuHandle.ptr += handleIndex * m_descriptorSize;

				handle.SetCPUHandle(cpuHandle);
				handle.SetGPUHandle(gpuHandle);
				handle.SetHeapIndex(handleIndex);

				m_allocateIndexs.push_back(handleIndex);

				++m_size;

				return true;
			}

			void DescriptorHeapPage_DX12::FreeHandle(DescriptorHeapHandle_DX12 handle)
			{
				if (std::find(m_freeSlots.begin(), m_freeSlots.end(), handle.HandleIndex) != m_freeSlots.end())
				{
					IS_CORE_ERROR("[DescriptorHeapPage_DX12::FreeHandle] Trying to free a handle which is already free.");
					return;
				}
				m_freeSlots.push_back(handle.HandleIndex);
				--m_size;
			}

			void DescriptorHeapPage_DX12::Reset()
			{
				m_freeSlots.clear();
				m_allocateIndexs.clear();
				m_size = 0;
			}


			/// <summary>
			/// DescriptorHeap_DX12
			/// </summary>
			/// <param name="heapType"></param>
			void DescriptorHeap_DX12::Create(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
			{
				m_heapType = heapType;
				AddNewHeap();
			}

			DescriptorHeapHandle_DX12 DescriptorHeap_DX12::GetNewHandle()
			{
				DescriptorHeapHandle_DX12 handle = { };
				m_heaps.back().GetNewHandle(handle);
				return handle;
			}

			bool DescriptorHeap_DX12::FindDescriptor(const Descriptor& descriptor, DescriptorHeapHandle_DX12& handle)
			{
				if (m_heaps.size() == 0 || m_heaps.back().IsFull())
				{
					assert(false);
					AddNewHeap();
				}

				const u64 descHash = descriptor.GetHash(true);
				auto descItr = m_descriptorHashToHandleIndex.find(descHash);
				if (descItr != m_descriptorHashToHandleIndex.end())
				{
					// Found descriptor hash.
					handle = m_descriptorHeapHandle.find(descItr->second)->second;
					return true;
				}
				m_heaps.back().GetNewHandle(handle);

				RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(descriptor.BufferView.GetBuffer());

				D3D12_CONSTANT_BUFFER_VIEW_DESC bufferViewDesc = {};
				bufferViewDesc.BufferLocation = bufferDX12->GetResouce()->GetGPUVirtualAddress() + descriptor.BufferView.GetOffset();
				bufferViewDesc.SizeInBytes = descriptor.BufferView.GetSize();
				m_context->GetDevice()->CreateConstantBufferView(&bufferViewDesc, handle.CPUPtr);

				m_descriptorHashToHandleIndex[descHash] = handle.HandleIndex;
				m_descriptorHeapHandle[handle.HandleIndex] = handle;

				return false;
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

			void DescriptorHeap_DX12::AddNewHeap()
			{
				DescriptorHeapPage_DX12 heap(256, m_heapType, m_context);
				m_heaps.push_back(heap);
			}

			///// <summary>
			///// RHI_Descriptor_DX12
			///// </summary>
			//void RHI_Descriptor_DX12::Release()
			//{
			//}

			//bool RHI_Descriptor_DX12::ValidResouce()
			//{
			//	return false;
			//}

			//void RHI_Descriptor_DX12::SetName(std::wstring name)
			//{
			//}

		//	bool DescriptorAllocator_DX12::SetupDescriptors()
		//	{
		//		for (const auto& set : m_descriptors)
		//		{
		//			const std::vector<Descriptor>& descriptos = set.second;
		//			for (const auto& desc : descriptos)
		//			{
		//				D3D12_DESCRIPTOR_HEAP_TYPE descTypeDX12 = DescriptorTypeToDX12(desc.Type);
		//				if (m_heaps.find(descTypeDX12) == m_heaps.end())
		//				{
		//					// Create intital heap if needed.
		//					m_heaps[descTypeDX12].SetRenderContext(m_context);
		//					m_heaps[descTypeDX12].Create(descTypeDX12);
		//				}

		//				DescriptorHeapHandle_DX12 handle = {};
		//				m_heaps[descTypeDX12].FindDescriptor(desc, handle);
		//			}
		//		}
		//		return true;
		//	}

		//	std::vector<ID3D12DescriptorHeap*> DescriptorAllocator_DX12::GetHeaps() const
		//	{
		//		std::vector<ID3D12DescriptorHeap*> result;
		//		for (const auto& pair : m_heaps)
		//		{
		//			for (const auto& heap : pair.second.GetHeaps())
		//			{
		//				result.push_back(heap.GetHeap());
		//			}
		//		}
		//		return result;
		//	}

		//	void DescriptorAllocator_DX12::SetDescriptors(CommandList_DX12* cmdList)
		//	{
		//		for (const auto& set : m_descriptors)
		//		{
		//			const std::vector<Descriptor>& descriptos = set.second;
		//			for (const auto& desc : descriptos)
		//			{
		//				D3D12_DESCRIPTOR_HEAP_TYPE descTypeDX12 = DescriptorTypeToDX12(desc.Type);

		//				DescriptorHeapHandle_DX12 handle = {};
		//				assert(m_heaps.find(descTypeDX12)->second.FindDescriptor(desc, handle));
		//				//cmdList->GetCommandBuffer()->SetGraphicsRootDescriptorTable(desc.Binding, handle.GPUPtr);
		//			}
		//		}
		//	}

		//	void DescriptorAllocator_DX12::BindTempConstentBuffer(ID3D12GraphicsCommandList* cmdList, RHI_BufferView bufferView, u32 rootParameterIndex)
		//	{
		//		if (m_heaps.find(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) == m_heaps.end())
		//		{
		//			// Create intital heap if needed.
		//			m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].SetRenderContext(m_context);
		//			m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//		}

		//		DescriptorHeapHandle_DX12 handle = m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetNewHandle();

		//		RHI_Buffer_DX12* buffer_dx12 = static_cast<RHI_Buffer_DX12*>(bufferView.GetBuffer());

		//		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { };
		//		cbvDesc.BufferLocation = buffer_dx12->GetResouce()->GetGPUVirtualAddress() + bufferView.GetOffset();
		//		cbvDesc.SizeInBytes = AlignUp(bufferView.GetSize(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		//		m_context->GetDevice()->CreateConstantBufferView(&cbvDesc, handle.CPUPtr);

		//		cmdList->SetGraphicsRootConstantBufferView(rootParameterIndex, cbvDesc.BufferLocation);
		//	}

		//	void DescriptorAllocator_DX12::SetDescriptorTables(RHI_CommandList_DX12* cmdList)
		//	{
		//		for (const auto& sets : m_descriptors)
		//		{
		//			int rootParaemterIndex = 0;
		//			for (const Descriptor& descriptor : sets.second)
		//			{
		//				D3D12_DESCRIPTOR_HEAP_TYPE heapType = DescriptorTypeToDX12(descriptor.Type);
		//				if (m_heaps.find(heapType) == m_heaps.end())
		//				{
		//					// Create intital heap if needed.
		//					m_heaps[heapType].SetRenderContext(m_context);
		//					m_heaps[heapType].Create(heapType);
		//				}

		//				u64 hash = descriptor.GetHash(true);
		//				DescriptorHeapHandle_DX12 handle = m_boundDescriptorsHandle[hash];

		//				if (m_boundDescriptors[sets.first][descriptor.Binding] != hash)
		//				{
		//					handle = m_heaps[heapType].GetNewHandle();

		//					if (descriptor.Type == DescriptorType::Unifom_Buffer)
		//					{
		//						RHI_Buffer_DX12* const buffer_dx12 = static_cast<RHI_Buffer_DX12*>(descriptor.BufferView.GetBuffer());

		//						D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { };
		//						cbvDesc.BufferLocation = buffer_dx12->GetResouce()->GetGPUVirtualAddress() + descriptor.BufferView.GetOffset();
		//						cbvDesc.SizeInBytes = AlignUp(descriptor.BufferView.GetSize(), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		//						m_context->GetDevice()->CreateConstantBufferView(&cbvDesc, handle.CPUPtr);
		//					}
		//					else if (descriptor.Type == DescriptorType::Combined_Image_Sampler)
		//					{
		//						RHI_Texture_DX12* const texture = static_cast<RHI_Texture_DX12*>(descriptor.Texture);

		//						cmdList->ResourceBarrierImage(texture->GetResouce(), D3D12_RESOURCE_STATE_COPY_DEST,
		//							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		//						D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
		//						srvDesc.Format = PixelFormatToDX12(
		//							PixelFormatExtensions::IsDepth(texture->GetFormat()) ?
		//							(texture->GetFormat() == PixelFormat::D32_Float ?
		//								PixelFormat::R32_Float : texture->GetFormat())
		//							: texture->GetFormat());
		//						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		//						srvDesc.Texture2D.MipLevels = 1;
		//						srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//						m_context->GetDevice()->CreateShaderResourceView(texture->GetResouce(), &srvDesc, handle.CPUPtr);
		//					}

		//					m_boundDescriptors[sets.first][descriptor.Binding] = hash;
		//					m_boundDescriptorsHandle[hash] = handle;
		//				}
		//				m_descrptorTables.push_back(std::make_pair(rootParaemterIndex, handle));
		//				++rootParaemterIndex;
		//			}
		//		}
		//	}

		//	void DescriptorAllocator_DX12::BindDescriptorTables(ID3D12GraphicsCommandList* cmdList)
		//	{
		//		for (const auto& pair : m_descrptorTables)
		//		{
		//			cmdList->SetGraphicsRootDescriptorTable(pair.first, pair.second.GetGPUHandle());
		//		}
		//	}

		//	void DescriptorAllocator_DX12::SetRenderContext(RenderContext* context)
		//	{
		//		m_context = static_cast<RenderContext_DX12*>(context);

		//		//D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//		//m_heaps[heapType].SetRenderContext(m_context);
		//		//m_heaps[heapType].Create(heapType);
		//		//
		//		//D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		//		//m_heaps[heapType].SetRenderContext(m_context);
		//		//m_heaps[heapType].Create(heapType);
		//		//
		//		//D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		//		//m_heaps[heapType].SetRenderContext(m_context);
		//		//m_heaps[heapType].Create(heapType);
		//		//
		//		//D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		//		//m_heaps[heapType].SetRenderContext(m_context);
		//		//m_heaps[heapType].Create(heapType);
		//	}

		//	bool DescriptorAllocator_DX12::GetDescriptors(std::vector<RHI_Descriptor*>& descriptors)
		//	{
		//		return false;
		//	}

		//	void DescriptorAllocator_DX12::Reset()
		//	{
		//		// Release all buffer views.
		//		for (auto& heap : m_heaps)
		//		{
		//			heap.second.Reset();
		//		}
		//		m_descrptorTables.clear();
		//		m_boundDescriptors.clear();
		//		m_boundDescriptorsHandle.clear();
		//	}

		//	void DescriptorAllocator_DX12::Destroy()
		//	{
		//		m_heaps.clear();
		//	}
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)