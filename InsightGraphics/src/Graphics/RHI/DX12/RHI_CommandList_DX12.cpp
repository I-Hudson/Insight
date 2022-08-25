#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			/// <summary>
			/// RHI_CommandList_DX12
			/// </summary>
			/// <param name="count"></param>
			/// <param name="barriers"></param>
			void RHI_CommandList_DX12::ResourceBarrier(int count, D3D12_RESOURCE_BARRIER* barriers)
			{
				if (m_commandList)
				{
					m_commandList->ResourceBarrier(count, barriers);
				}
			}

			void RHI_CommandList_DX12::ResourceBarrierImage(ID3D12Resource* resource, D3D12_RESOURCE_STATES srcState, D3D12_RESOURCE_STATES dstState)
			{
				D3D12_RESOURCE_BARRIER barrierdesc = {};
				barrierdesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrierdesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrierdesc.Transition.pResource = resource;
				barrierdesc.Transition.StateBefore = srcState;
				barrierdesc.Transition.StateAfter = dstState;
				barrierdesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				ResourceBarrier(1, &barrierdesc);
			}

			void RHI_CommandList_DX12::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float* clearColour, int numRects, D3D12_RECT* rects)
			{
				if (m_commandList)
				{
					m_commandList->ClearRenderTargetView(rtvHandle, clearColour, numRects, rects);
				}
			}

			void RHI_CommandList_DX12::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, int Stencil, int NumRects, D3D12_RECT* rects)
			{
				if (m_commandList)
				{
					m_commandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, rects);
				}
			}

			void RHI_CommandList_DX12::OMSetRenderTargets(int count, D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, bool RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor)
			{
				if (m_commandList)
				{
					m_commandList->OMSetRenderTargets(count, rtvHandles, RTsSingleHandleToDescriptorRange, depthStencilDescriptor);
				}
			}

			void RHI_CommandList_DX12::SetDescriptorHeaps(int count, ID3D12DescriptorHeap** heaps)
			{
				if (m_commandList)
				{
					m_commandList->SetDescriptorHeaps(count, heaps);
				}
			}

			void RHI_CommandList_DX12::SetDescriptorHeaps(std::vector<ID3D12DescriptorHeap*> heaps)
			{
				SetDescriptorHeaps((int)heaps.size(), heaps.data());
			}

			ID3D12GraphicsCommandList* RHI_CommandList_DX12::GetCommandList() const
			{
				return m_commandList.Get();
			}

			void RHI_CommandList_DX12::Reset()
			{
				RHI_CommandList::Reset();
				if (m_commandList && m_allocator)
				{
					m_commandList->Reset(m_allocator->GetAllocator(), nullptr);
				}
			}

			void RHI_CommandList_DX12::Close()
			{
				if (m_commandList)
				{
					m_commandList->Close();
				}
			}

			void RHI_CommandList_DX12::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Buffer_DX12* dstDX12 = static_cast<RHI_Buffer_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);
				m_commandList->CopyBufferRegion(dstDX12->GetResouce(), offset, srcDX12->GetResouce(), 0, src->GetSize());
			}

			void RHI_CommandList_DX12::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src)
			{
				RHI_Texture_DX12* dstDX12 = static_cast<RHI_Texture_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);

				u64 requriedSize = 0;
				std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 1> layouts;
				std::array<u64, 1> rowSizeInBytes;
				std::array<UINT, 1> numRows;
				RenderContextDX12()->GetDevice()->GetCopyableFootprints(
					&dstDX12->GetResouce()->GetDesc(),
					0,
					1, 
					0, 
					layouts.data(),
					numRows.data(),
					rowSizeInBytes.data(),
					&requriedSize);

				assert(requriedSize == (dstDX12->GetWidth() * dstDX12->GetHeight() * 4));
				assert(requriedSize == srcDX12->GetSize());

				CD3DX12_TEXTURE_COPY_LOCATION Dst(dstDX12->GetResouce(), 0);
				CD3DX12_TEXTURE_COPY_LOCATION Src(srcDX12->GetResouce(), layouts[0]);
				m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
			}

			void RHI_CommandList_DX12::Release()
			{
				if (m_commandList)
				{
					m_commandList.Reset();
					m_commandList = nullptr;
				}
			}

			bool RHI_CommandList_DX12::ValidResouce()
			{
				return m_commandList;
			}

			void RHI_CommandList_DX12::SetName(std::wstring name)
			{
				if (m_commandList)
				{
					m_commandList->SetName(name.c_str());
				}
			}

			void RHI_CommandList_DX12::BeginRenderpass(RenderpassDescription renderDescription)
			{
				IS_PROFILE_FUNCTION();

			}

			void RHI_CommandList_DX12::EndRenderpass()
			{
				IS_PROFILE_FUNCTION();

			}

			void RHI_CommandList_DX12::SetPipeline(PipelineStateObject pso)
			{
				IS_PROFILE_FUNCTION();
				m_pso = pso;
				//FrameResourceDX12()->DescriptorAllocator.SetPipeline(m_pso);
			}

			void RHI_CommandList_DX12::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y)
			{
				IS_PROFILE_FUNCTION();
				assert(m_commandList);
				D3D12_VIEWPORT viewports[1] = { D3D12_VIEWPORT{ x, y, width, height, minDepth, maxDepth } };
				m_commandList->RSSetViewports(1, viewports);
			}

			void RHI_CommandList_DX12::SetScissor(int x, int y, int width, int height)
			{
				IS_PROFILE_FUNCTION();
				assert(m_commandList);
				D3D12_RECT rects[1] = { D3D12_RECT{ x, y, width, height } };
				m_commandList->RSSetScissorRects(1, rects);
			}

			void RHI_CommandList_DX12::SetVertexBuffer(RHI_Buffer* buffer)
			{
				IS_PROFILE_FUNCTION();
				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(buffer);
				const D3D12_VERTEX_BUFFER_VIEW views[] = { D3D12_VERTEX_BUFFER_VIEW{bufferDX12->GetResouce()->GetGPUVirtualAddress(), 
					(UINT)bufferDX12->GetSize(),
					(UINT)bufferDX12->GetStride() }};
				m_commandList->IASetVertexBuffers(0, 1, views);
			}

			void RHI_CommandList_DX12::SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type)
			{
				IS_PROFILE_FUNCTION();
				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(buffer);
				const D3D12_INDEX_BUFFER_VIEW view = { bufferDX12->GetResouce()->GetGPUVirtualAddress(), 
					(UINT)bufferDX12->GetSize(),  
					DXGI_FORMAT_R32_UINT };
				m_commandList->IASetIndexBuffer(&view);
			}

			void RHI_CommandList_DX12::Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance)
			{
				IS_PROFILE_FUNCTION();
				assert(m_commandList);
				m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
			}

			void RHI_CommandList_DX12::DrawIndexed(int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance)
			{
				IS_PROFILE_FUNCTION();
				assert(m_commandList);
				m_commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
			}

			void RHI_CommandList_DX12::BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout)
			{
				IS_PROFILE_FUNCTION();
				
				ID3D12PipelineState* pipeline = RenderContextDX12()->GetPipelineStateObjectManager().GetOrCreatePSO(pso);
				m_commandList->SetPipelineState(pipeline);

				RHI_DescriptorLayout_DX12* layout_DX12 = static_cast<RHI_DescriptorLayout_DX12*>(layout);
				m_commandList->SetGraphicsRootSignature(layout_DX12->GetRootSignature());
				m_commandList->IASetPrimitiveTopology(PrimitiveTopologyTypeToDX12(m_activePSO.PrimitiveTopologyType));
			}

			bool RHI_CommandList_DX12::BindDescriptorSets()
			{
				IS_PROFILE_FUNCTION();
				bool result = true;// m_frameResouces->DescriptorAllocator.SetupDescriptors();
				//FrameResourceDX12()->DescriptorAllocator.BindTempConstentBuffer(GetCommandList(), FrameResourceDX12()->DescriptorAllocator.GetDescriptor(0, 0).BufferView, 0);
				
				//FrameResourceDX12()->DescriptorAllocator.SetDescriptorTables(this);

				//std::vector<ID3D12DescriptorHeap*> descriptors = FrameResourceDX12()->DescriptorAllocator.GetHeaps();
				//if (result && descriptors.size() > 0)
				{
					// Set our descriptor heaps.
					//m_commandList->SetDescriptorHeaps(static_cast<UINT>(descriptors.size()), descriptors.data());

					// Set all our descriptors tables.
					//FrameResourceDX12()->DescriptorAllocator.BindDescriptorTables(GetCommandList());
				}
				return true;
			}

			RenderContext_DX12* RHI_CommandList_DX12::RenderContextDX12()
			{
				IS_PROFILE_FUNCTION();
				assert(m_context);
				return static_cast<RenderContext_DX12*>(m_context);
			}

			FrameResource_DX12* RHI_CommandList_DX12::FrameResourceDX12()
			{
				IS_PROFILE_FUNCTION();
				assert(m_frameResouces);
				return static_cast<FrameResource_DX12*>(m_frameResouces);
			}



			/// <summary>
			/// RHI_CommandListAllocator_DX12
			/// </summary>
			/// <param name="context"></param>
			void RHI_CommandListAllocator_DX12::Create(RenderContext* context)
			{
				m_context = static_cast<RenderContext_DX12*>(context);
				ThrowIfFailed(m_context->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)));
			}

			RHI_CommandList* RHI_CommandListAllocator_DX12::GetCommandList()
			{
				if (m_freeLists.size() > 0)
				{
					RHI_CommandList* list = *m_freeLists.begin();
					m_freeLists.erase(m_freeLists.begin());
					m_allocLists.insert(list);
					return list;
				}

				RHI_CommandList_DX12* list = static_cast<RHI_CommandList_DX12*>(RHI_CommandList::New());
				list->m_context = m_context;
				list->m_allocator = this;
				ThrowIfFailed(m_context->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr, IID_PPV_ARGS(&list->m_commandList)));

				m_allocLists.insert(list);
				return list;
			}

			RHI_CommandList* RHI_CommandListAllocator_DX12::GetSingleSubmitCommandList()
			{
				ComPtr<ID3D12CommandAllocator> allocator;
				ThrowIfFailed(m_context->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
				
				RHI_CommandList_DX12* list = static_cast<RHI_CommandList_DX12*>(RHI_CommandList::New());
				ThrowIfFailed(m_context->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list->m_commandList)));
				
				m_singleUseCommandLists[list] = std::make_pair(list, allocator);

				return list;
			}

			void RHI_CommandListAllocator_DX12::ReturnSingleSubmitCommandList(RHI_CommandList* cmdList)
			{
				auto itr = m_singleUseCommandLists.find(cmdList);
				if (itr == m_singleUseCommandLists.end())
				{
					IS_CORE_ERROR("[RHI_CommandListAllocator_DX12::ReturnSingleSubmitCommandList]");
					return;
				}

				cmdList->Release();
				DeleteTracked(cmdList);
				m_singleUseCommandLists.erase(itr);
			}

			void RHI_CommandListAllocator_DX12::Reset()
			{
				m_allocator->Reset();
				while (m_allocLists.size() > 0)
				{
					RHI_CommandList* list = *m_allocLists.begin();
					list->Reset();

					m_freeLists.insert(list);
					m_allocLists.erase(list);
				}
			}

			void RHI_CommandListAllocator_DX12::Release()
			{
				if (m_allocator)
				{
					Reset();

					for (auto list : m_allocLists)
					{
						list->Release();
						DeleteTracked(list);
					}
					for (auto list : m_freeLists)
					{
						list->Release();
						DeleteTracked(list);
					}
					m_allocLists.clear();
					m_freeLists.clear();

					m_allocator.Reset();
					m_allocator = nullptr;
				}
			}

			bool RHI_CommandListAllocator_DX12::ValidResouce()
			{
				return m_allocator;
			}

			void RHI_CommandListAllocator_DX12::SetName(std::wstring name)
			{
				if (m_allocator)
				{
					m_allocator->SetName(name.c_str());
				}
			}
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)