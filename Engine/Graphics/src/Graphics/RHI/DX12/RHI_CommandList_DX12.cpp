#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"
#include "Graphics/RHI/DX12/RHI_Pipeline_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <WinPixEventRuntime/pix3.h>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			void RHI_CommandList_DX12::Create(RenderContext* context)
			{
				m_context = context;
				m_contextDX12 = static_cast<RenderContext_DX12*>(m_context);
			}

			void RHI_CommandList_DX12::PipelineBarrier(Graphics::PipelineBarrier barrier)
			{
				D3D12_BARRIER_SYNC beforeSync = D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE;
				if ((barrier.SrcStage & (u32)PipelineStageFlagBits::TopOfPipe) == 0)
				{
					// If SrcStage is not Top of pipe then get our stage value.
					beforeSync = PipelineStageFlagsToDX12(barrier.SrcStage);
				}
				D3D12_BARRIER_SYNC afterSync = D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE;
				if ((barrier.DstStage & (u32)PipelineStageFlagBits::BottomOfPipe) == 0)
				{
					// If DstStage is not bottom of pipe then get our stage value.
					afterSync = PipelineStageFlagsToDX12(barrier.DstStage);
				}

				std::vector<D3D12_BUFFER_BARRIER> bufferBarriers;
				for (const BufferBarrier& bufferBarrier : barrier.BufferBarriers)
				{
					RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(bufferBarrier.Buffer);

					D3D12_BARRIER_ACCESS accessBefore = AccessFlagsToDX12(bufferBarrier.SrcAccessFlags);
					D3D12_BARRIER_ACCESS accessAfter = AccessFlagsToDX12(bufferBarrier.SrcAccessFlags);

					ASSERT(beforeSync == D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE
						&& accessBefore == D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_NO_ACCESS);
					ASSERT(afterSync == D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE
						&& accessAfter == D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_NO_ACCESS);

					bufferBarriers.push_back(CD3DX12_BUFFER_BARRIER(
						beforeSync,
						afterSync,
						accessBefore,
						accessAfter,
						bufferDX12->GetResource()
					));
				}

				std::vector<D3D12_TEXTURE_BARRIER> imageBarriers;
				for (const ImageBarrier& imageBarrier : barrier.ImageBarriers)
				{
					RHI_Texture_DX12* textureDX12 = static_cast<RHI_Texture_DX12*>(imageBarrier.Image);

					D3D12_BARRIER_ACCESS accessBefore = AccessFlagsToDX12(imageBarrier.SrcAccessFlags);
					D3D12_BARRIER_ACCESS accessAfter = AccessFlagsToDX12(imageBarrier.DstAccessFlags);

					if (beforeSync == D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE)
					{
						ASSERT(accessBefore == D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_NO_ACCESS)
					}
					if (afterSync == D3D12_BARRIER_SYNC::D3D12_BARRIER_SYNC_NONE)
					{
						ASSERT(accessAfter == D3D12_BARRIER_ACCESS::D3D12_BARRIER_ACCESS_NO_ACCESS)
					}

					D3D12_BARRIER_SUBRESOURCE_RANGE subresources = CD3DX12_BARRIER_SUBRESOURCE_RANGE(
						imageBarrier.SubresourceRange.BaseMipLevel,
						imageBarrier.SubresourceRange.LevelCount,
						imageBarrier.SubresourceRange.BaseArrayLayer,
						imageBarrier.SubresourceRange.LayerCount
					);
					imageBarriers.push_back(CD3DX12_TEXTURE_BARRIER(
						beforeSync,
						afterSync,
						accessBefore,
						accessAfter,
						ImageLayoutToDX12(imageBarrier.OldLayout),
						ImageLayoutToDX12(imageBarrier.NewLayout),
						textureDX12->GetResource(),
						subresources
					));

					imageBarrier.Image->SetLayout(imageBarrier.NewLayout);					
				}
				PipelineBarrier(bufferBarriers, imageBarriers);
			}

			void RHI_CommandList_DX12::PipelineBarrier(std::vector<D3D12_BUFFER_BARRIER> const& bufferMemoryBarrier, std::vector<D3D12_TEXTURE_BARRIER> const& imageMemoryBarrier)
			{
				CD3DX12_BARRIER_GROUP barrierGoup[] =
				{
					CD3DX12_BARRIER_GROUP(static_cast<UINT>(bufferMemoryBarrier.size()), bufferMemoryBarrier.data()),
					CD3DX12_BARRIER_GROUP(static_cast<UINT>(imageMemoryBarrier.size()), imageMemoryBarrier.data()),
				};
				m_commandList->Barrier(ARRAY_COUNT(barrierGoup), barrierGoup);
			}

			void RHI_CommandList_DX12::PipelineBarrierBuffer(std::vector<D3D12_BUFFER_BARRIER> const& bufferMemoryBarrier)
			{
				PipelineBarrier(bufferMemoryBarrier, {});
			}

			void RHI_CommandList_DX12::PipelineBarrierImage(std::vector<D3D12_TEXTURE_BARRIER> const& imageMemoryBarrier)
			{
				PipelineBarrier({}, imageMemoryBarrier);
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
				return m_commandList;
			}

			void RHI_CommandList_DX12::Reset()
			{
				RHI_CommandList::Reset();
			
				if (m_commandList && m_allocator)
				{
					m_commandList->Reset(m_allocator->GetAllocator(), nullptr);
					m_state = RHI_CommandListStates::Recording;
				}
			}

			void RHI_CommandList_DX12::Close()
			{
				if (m_commandList)
				{
					ASSERT(m_state == RHI_CommandListStates::Recording);
					m_commandList->Close();
					m_state = RHI_CommandListStates::Ended;
				}
			}

			void RHI_CommandList_DX12::CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Buffer_DX12* dstDX12 = static_cast<RHI_Buffer_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);
				m_commandList->CopyBufferRegion(dstDX12->GetResource(), offset, srcDX12->GetResource(), 0, src->GetSize());
			}

			void RHI_CommandList_DX12::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset)
			{
				RHI_Texture_DX12* dstDX12 = static_cast<RHI_Texture_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);

				u64 requriedSize = 0;
				std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 1> layouts;
				std::array<u64, 1> rowSizeInBytes;
				std::array<UINT, 1> numRows;
				m_contextDX12->GetDevice()->GetCopyableFootprints(
					&dstDX12->GetResource()->GetDesc(),
					0,
					1, 
					0, 
					layouts.data(),
					numRows.data(),
					rowSizeInBytes.data(),
					&requriedSize);

				assert(requriedSize == (dstDX12->GetWidth() * dstDX12->GetHeight() * 4));
				assert(requriedSize == srcDX12->GetSize());

				CD3DX12_TEXTURE_COPY_LOCATION Dst(dstDX12->GetResource(), 0);
				CD3DX12_TEXTURE_COPY_LOCATION Src(srcDX12->GetResource(), layouts[0]);
				m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
			}

			void RHI_CommandList_DX12::Release()
			{
				if (m_commandList)
				{
					m_commandList->Release();
					m_commandList = nullptr;
				}
			}

			bool RHI_CommandList_DX12::ValidResource()
			{
				return m_commandList;
			}

			void RHI_CommandList_DX12::SetName(std::string name)
			{
				if (m_commandList)
				{
					m_contextDX12->SetObjectName(name, m_commandList);
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
				///FrameResourceDX12()->DescriptorAllocator.SetPipeline(m_pso);
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

			void RHI_CommandList_DX12::SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor)
			{
				IS_CORE_INFO("[ RHI_CommandList_DX12::SetDepthBias] Not implemented.");
			}

			void RHI_CommandList_DX12::SetLineWidth(float width)
			{
				IS_CORE_INFO("[ RHI_CommandList_DX12::SetLineWidth] Not implemented.");
			}

			void RHI_CommandList_DX12::SetVertexBuffer(RHI_Buffer* buffer)
			{
				IS_PROFILE_FUNCTION();
				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(buffer);
				const D3D12_VERTEX_BUFFER_VIEW views[] = { D3D12_VERTEX_BUFFER_VIEW{bufferDX12->GetResource()->GetGPUVirtualAddress(), 
					(UINT)bufferDX12->GetSize(),
					(UINT)bufferDX12->GetStride() }};
				m_commandList->IASetVertexBuffers(0, 1, views);
			}

			void RHI_CommandList_DX12::SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type)
			{
				IS_PROFILE_FUNCTION();
				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(buffer);
				const D3D12_INDEX_BUFFER_VIEW view = { bufferDX12->GetResource()->GetGPUVirtualAddress(), 
					(UINT)bufferDX12->GetSize(),  
					DXGI_FORMAT_R32_UINT };
				m_commandList->IASetIndexBuffer(&view);
			}

			void RHI_CommandList_DX12::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				ASSERT(m_commandList);
				m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
			}

			void RHI_CommandList_DX12::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw())
				{
					{
						ASSERT(m_commandList);
						m_commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
					}
				}
			}

			void RHI_CommandList_DX12::BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout)
			{
				IS_PROFILE_FUNCTION();
				
				ID3D12PipelineState* pipeline = static_cast<RHI_Pipeline_DX12*>(m_contextDX12->GetPipelineManager().GetOrCreatePSO(pso))->GetPipeline();
				m_commandList->SetPipelineState(pipeline);

				
				RHI_PipelineLayout_DX12* pipelineLayout = static_cast<RHI_PipelineLayout_DX12*>(m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso));
				m_commandList->SetGraphicsRootSignature(pipelineLayout->GetRootSignature());
				m_commandList->IASetPrimitiveTopology(PrimitiveTopologyToDX12(m_activePSO.PrimitiveTopologyType));
			}

			void RHI_CommandList_DX12::BeginTimeBlock(const std::string& blockName)
			{
				BeginTimeBlock(blockName, glm::vec4(1, 1, 1, 1));
			}

			void RHI_CommandList_DX12::BeginTimeBlock(const std::string& blockName, glm::vec4 colour)
			{
				colour.x = std::max(0.0f, std::min(1.0f, colour.x));
				colour.y = std::max(0.0f, std::min(1.0f, colour.y));
				colour.z = std::max(0.0f, std::min(1.0f, colour.z));
				PIXBeginEvent(m_commandList, PIX_COLOR(static_cast<BYTE>(colour.x * 255), static_cast<BYTE>(colour.y * 255), static_cast<BYTE>(colour.z * 255)), blockName.c_str());
			}

			void RHI_CommandList_DX12::EndTimeBlock()
			{
				PIXEndEvent();
			}

			bool RHI_CommandList_DX12::BindDescriptorSets()
			{
				IS_PROFILE_FUNCTION();

				ID3D12DescriptorHeap* heaps[] = 
				{
					m_contextDX12->GetFrameDescriptorHeapGPU().GetHeap(0)
				};
				m_commandList->SetDescriptorHeaps(ARRAY_COUNT(heaps), heaps);
				std::vector<DescriptorHeapHandle_DX12> descriptorHeapHandles = m_descriptorAllocator->GetDescriptorHeapHandles();



				return true;
			}

			void RHI_CommandList_DX12::SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout)
			{
				FAIL_ASSERT();
			}

			//// <summary>
			//// RHI_CommandListAllocator_DX12
			//// </summary>
			//// <param name="context"></param>
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
					list->Reset();
					return list;
				}

				RHI_CommandList_DX12* list = static_cast<RHI_CommandList_DX12*>(RHI_CommandList::New());
				list->Create(m_context);
				list->m_allocator = this;
				ThrowIfFailed(m_context->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr, IID_PPV_ARGS(&list->m_commandList)));
				list->m_state = RHI_CommandListStates::Recording;

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

			bool RHI_CommandListAllocator_DX12::ValidResource()
			{
				return m_allocator;
			}

			void RHI_CommandListAllocator_DX12::SetName(std::string name)
			{
				if (m_allocator)
				{
					m_context->SetObjectName(name, m_allocator.Get());
				}
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)