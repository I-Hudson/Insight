#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"
#include "Graphics/RHI/DX12/RHI_Pipeline_DX12.h"
#include "Graphics/RHI/DX12/RHI_Sampler_DX12.h"

#include "Graphics/RenderTarget.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <WinPixEventRuntime/pix3.h>

#include <glm/gtc/type_ptr.hpp>

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
#ifdef DX12_ENHANCED_BARRIERS
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
#else
				std::vector<D3D12_RESOURCE_BARRIER> resouceBarriers;
				for (const BufferBarrier& bufferBarrier : barrier.BufferBarriers)
				{
					RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(bufferBarrier.Buffer);

					D3D12_RESOURCE_STATES oldState = bufferDX12->m_currentResouceState;
					D3D12_RESOURCE_STATES newState = BufferTypeToDX12ResourceState(bufferBarrier.NewLayout);
					if (oldState != newState
						&& bufferDX12->GetResource() != nullptr)
					{
						resouceBarriers.push_back(
							CD3DX12_RESOURCE_BARRIER::Transition(
								bufferDX12->GetResource(),
								oldState,
								newState));
						bufferDX12->m_currentResouceState = newState;
					}
				}

				for (const ImageBarrier& imageBarrier : barrier.ImageBarriers)
				{
					RHI_Texture_DX12* textureDX12 = static_cast<RHI_Texture_DX12*>(imageBarrier.Image);

					// Only transition a texture if the layout is different. If the new layout
					// is the same as the current layout then don't place a barrier for the texture
					// as no transition is needed.
					D3D12_RESOURCE_STATES oldState = ImageLayoutToDX12ResouceState(imageBarrier.OldLayout);
					D3D12_RESOURCE_STATES newState = ImageLayoutToDX12ResouceState(imageBarrier.NewLayout);
					if (oldState != newState
						&& textureDX12->GetResource() != nullptr)
					{
						resouceBarriers.push_back(
							CD3DX12_RESOURCE_BARRIER::Transition(
								textureDX12->GetResource(),
								oldState,
								newState));
						textureDX12->SetLayout(imageBarrier.NewLayout);
					}
				}

				PipelineResourceBarriers(resouceBarriers);
#endif
			}

#ifdef DX12_ENHANCED_BARRIERS

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
#endif

			void RHI_CommandList_DX12::PipelineResourceBarriers(std::vector<D3D12_RESOURCE_BARRIER> const& resourceBarriers)
			{
				if (resourceBarriers.size() > 0ull)
				{
					nvtx3::mark("PipelineResourceBarriers");
					m_commandList->ResourceBarrier(static_cast<UINT>(resourceBarriers.size()), resourceBarriers.data());
					++RenderStats::Instance().PipelineBarriers;
				}
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
					m_commandList->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, static_cast<UINT8>(Stencil), NumRects, rects);
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
			
				if (m_commandList && m_allocator
					&& (m_state == RHI_CommandListStates::Ended || m_state == RHI_CommandListStates::Submitted))
				{
					m_commandList->Reset(m_allocator->GetAllocator(), nullptr);
					m_state = RHI_CommandListStates::Recording;
				}

				m_boundResourceHeap = nullptr;
#ifdef DX12_REUSE_DESCRIPTOR_TABLES
				m_descriptorTableCache.clear();
#endif // DX12_REUSE_DESCRIPTOR_TABLES
#ifdef IS_DESCRIPTOR_MULTITHREAD_DX12
				m_subHeapRes = {};
				m_subHeapSampler = {};
#endif // IS_DESCRIPTOR_MULTITHREAD_DX12

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

			void RHI_CommandList_DX12::CopyBufferToBuffer(RHI_Buffer* dst, u64 dstOffset, RHI_Buffer* src, u64 srcOffset, u64 sizeInBytes)
			{
				BeginTimeBlock("CopyBufferToBuffer");

				RHI_Buffer_DX12* dstDX12 = static_cast<RHI_Buffer_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);

				std::vector<D3D12_RESOURCE_BARRIER> barriers = {};
				if (dstDX12->GetResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
				{
					barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(dstDX12->GetResource(), dstDX12->GetResourceState(), D3D12_RESOURCE_STATE_COPY_DEST));
					dstDX12->SetResourceState(D3D12_RESOURCE_STATE_COPY_DEST);
				}
				if (srcDX12->GetResourceState() != D3D12_RESOURCE_STATE_COPY_SOURCE)
				{
					barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(srcDX12->GetResource(), srcDX12->GetResourceState(), D3D12_RESOURCE_STATE_COPY_SOURCE));
					srcDX12->SetResourceState(D3D12_RESOURCE_STATE_COPY_SOURCE);
				}
				PipelineResourceBarriers(barriers);

				m_commandList->CopyBufferRegion(dstDX12->GetResource(), dstOffset, srcDX12->GetResource(), srcOffset, sizeInBytes);
			
				barriers = {};
				if (dstDX12->GetResourceState() != BufferTypeToDX12ResourceState(dstDX12->GetType()))
				{
					barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(dstDX12->GetResource(), dstDX12->GetResourceState(), BufferTypeToDX12ResourceState(dstDX12->GetType())));
					dstDX12->SetResourceState(BufferTypeToDX12ResourceState(dstDX12->GetType()));
				}
				if (srcDX12->GetResourceState() != BufferTypeToDX12ResourceState(srcDX12->GetType()))
				{
					barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(srcDX12->GetResource(), srcDX12->GetResourceState(), BufferTypeToDX12ResourceState(srcDX12->GetType())));
					srcDX12->SetResourceState(BufferTypeToDX12ResourceState(srcDX12->GetType()));
				}
				PipelineResourceBarriers(barriers);
				EndTimeBlock();
			}

			void RHI_CommandList_DX12::CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset)
			{
				BeginTimeBlock("CopyBufferToImage");
				RHI_Texture_DX12* dstDX12 = static_cast<RHI_Texture_DX12*>(dst);
				RHI_Buffer_DX12* srcDX12 = static_cast<RHI_Buffer_DX12*>(src);

				D3D12_RESOURCE_DESC desc = dstDX12->GetResource()->GetDesc();

				u64 requriedSize = 0;
				std::array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT, 1> layouts;
				std::array<u64, 1> rowSizeInBytes;
				std::array<UINT, 1> numRows;
				m_contextDX12->GetDevice()->GetCopyableFootprints(
					&desc,
					0,
					1, 
					0, 
					layouts.data(),
					numRows.data(),
					rowSizeInBytes.data(),
					&requriedSize);
				layouts[0].Offset = offset;

				CD3DX12_TEXTURE_COPY_LOCATION Dst(dstDX12->GetResource(), 0);
				CD3DX12_TEXTURE_COPY_LOCATION Src(srcDX12->GetResource(), layouts[0]);
				m_commandList->CopyTextureRegion(&Dst, 0, 0, 0, & Src, nullptr);
				EndTimeBlock();
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
				m_name = name;
			}

			void RHI_CommandList_DX12::BeginRenderpass(RenderpassDescription renderDescription)
			{
				IS_PROFILE_FUNCTION();

				std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetHandles;
				D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = { 0 };

				u32 idx = 0;
				for (const RHI_Texture* image : renderDescription.ColourAttachments)
				{
					const AttachmentDescription attachmentDescription = renderDescription.Attachments.at(idx);
					const RHI_Texture_DX12* textureDX12 = static_cast<const RHI_Texture_DX12*>(image);
					DescriptorHeapHandle_DX12 rtvHandle = textureDX12->GetSingleLayerRenderTargetHandle(attachmentDescription.Layer_Array_Index);
					ASSERT(rtvHandle.CPUPtr.ptr != 0);

					AttachmentDescription const& description = renderDescription.Attachments.at(idx);
					if (description.LoadOp == AttachmentLoadOp::Clear)
					{
						Maths::Vector4 const clearColour = textureDX12->GetClearColour();
						ClearRenderTargetView(rtvHandle.CPUPtr, &clearColour[0], 0, nullptr);
					}

					renderTargetHandles.push_back(rtvHandle.CPUPtr);
					++idx;
				}
				idx = 0;

				if (renderDescription.DepthStencil)
				{
					const RHI_Texture_DX12* textureDX12 = static_cast<const RHI_Texture_DX12*>(renderDescription.DepthStencil);
					DescriptorHeapHandle_DX12 dsHandle = textureDX12->GetSingleLayerDepthStencilHandle(renderDescription.DepthStencilAttachment.Layer_Array_Index);
					ASSERT(dsHandle.CPUPtr.ptr != 0);

					if (renderDescription.DepthStencilAttachment.LoadOp == AttachmentLoadOp::Clear)
					{
						ClearDepthStencilView(dsHandle.CPUPtr,
							D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
							RenderContext::Instance().IsRenderOptionsEnabled(RenderOptions::ReverseZ) ? 0.0f : 1.0f, 
							0, 
							0, 
							nullptr);
					}
					depthStencilHandle = dsHandle.CPUPtr;
				}

				OMSetRenderTargets(static_cast<int>(renderTargetHandles.size()), renderTargetHandles.data(), false, depthStencilHandle.ptr != 0 ? &depthStencilHandle : nullptr);
			}

			void RHI_CommandList_DX12::EndRenderpass()
			{
				IS_PROFILE_FUNCTION();

				m_boundVertexBufferView = { };
				m_boundIndexBufferView = { };
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
				ASSERT(m_commandList);
				D3D12_RECT rects[1] = { D3D12_RECT{ x, y, width, height } };
				m_commandList->RSSetScissorRects(1, rects);
			}

			void RHI_CommandList_DX12::SetLineWidth(float width)
			{
				IS_LOG_CORE_INFO("[ RHI_CommandList_DX12::SetLineWidth] Not implemented.");
			}

			void RHI_CommandList_DX12::SetVertexBuffer(const RHI_BufferView& bufferView)
			{
				IS_PROFILE_FUNCTION();
				if (!bufferView.IsValid() || bufferView == m_boundVertexBufferView)
				{
					return;
				}

				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(bufferView.GetBuffer());
				const D3D12_VERTEX_BUFFER_VIEW views[] = 
				{ 
					D3D12_VERTEX_BUFFER_VIEW
					{
						bufferDX12->GetResource()->GetGPUVirtualAddress() + bufferView.GetOffset(),
						(UINT)bufferDX12->GetSize(),
						(UINT)bufferDX12->GetStride() 
					}
				};
				m_commandList->IASetVertexBuffers(bufferView.Vertex.Slot, 1, views);
				m_boundVertexBufferView = bufferView;
				++RenderStats::Instance().VertexBufferBindings;
			}

			void RHI_CommandList_DX12::SetIndexBuffer(const RHI_BufferView& bufferView, const IndexType index_type)
			{
				IS_PROFILE_FUNCTION();
				if (!bufferView.IsValid() || bufferView == m_boundIndexBufferView)
				{
					return;
				}

				const RHI_Buffer_DX12* bufferDX12 = static_cast<RHI_Buffer_DX12*>(bufferView.GetBuffer());
				const D3D12_INDEX_BUFFER_VIEW view = 
				{ 
					bufferDX12->GetResource()->GetGPUVirtualAddress() + bufferView.GetOffset(),
					(UINT)bufferDX12->GetSize(),  
					IndexTypeToDX12(index_type)
				};
				m_commandList->IASetIndexBuffer(&view);
				m_boundIndexBufferView = bufferView;
				++RenderStats::Instance().IndexBufferBindings;
			}

			void RHI_CommandList_DX12::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw(GPUQueue::GPUQueue_Graphics))
				{
					{
						IS_PROFILE_SCOPE("DrawInstanced");
						m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
						++RenderStats::Instance().DrawCalls;
					}
				}
			}

			void RHI_CommandList_DX12::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw(GPUQueue::GPUQueue_Graphics))
				{
					{
						IS_PROFILE_SCOPE("DrawIndexedInstanced");
						m_commandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
						++RenderStats::Instance().DrawIndexedCalls;
						RenderStats::Instance().DrawIndexedIndicesCount += indexCount;
					}
				}
			}

			void RHI_CommandList_DX12::Dispatch(const u32 threadGroupX, const u32 threadGroupY)
			{
				IS_PROFILE_FUNCTION();
				if (CanDraw(GPUQueue::GPUQueue_Compute))
				{
					{
						IS_PROFILE_SCOPE("Dispatch");
						m_commandList->Dispatch(threadGroupX, threadGroupY, 1);
					}
				}
			}

			void RHI_CommandList_DX12::BindPipeline(const PipelineStateObject& pso, RHI_DescriptorLayout* layout)
			{
				BindPipeline(pso, true);
			}

			void RHI_CommandList_DX12::BindPipeline(const PipelineStateObject& pso, bool clearDescriptors)
			{
				IS_PROFILE_FUNCTION();

				m_activePSO = pso;

				RHI_Pipeline_DX12* pipeline = static_cast<RHI_Pipeline_DX12*>(m_contextDX12->GetPipelineManager().GetOrCreatePSO(pso));
				m_commandList->SetPipelineState(pipeline->GetPipeline());

				RHI_PipelineLayout_DX12* pipelineLayout = static_cast<RHI_PipelineLayout_DX12*>(m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso));
				m_commandList->SetGraphicsRootSignature(pipelineLayout->GetRootSignature());
				m_commandList->IASetPrimitiveTopology(PrimitiveTopologyToDX12(m_activePSO.PrimitiveTopologyType));

				if (clearDescriptors)
				{
					m_descriptorAllocator->SetPipeline(pso);
#ifdef DX12_REUSE_DESCRIPTOR_TABLES
					m_boundDescriptorSets.clear();
					m_boundDescriptorSets.resize(m_descriptorAllocator->GetAllocatorDescriptorSets().size());
#endif // DX12_REUSE_DESCRIPTOR_TABLES
				}
			}

			void RHI_CommandList_DX12::BindPipeline(const ComputePipelineStateObject& pso)
			{
				m_activeComputePSO = pso;

				RHI_Pipeline_DX12* pipeline = static_cast<RHI_Pipeline_DX12*>(m_contextDX12->GetPipelineManager().GetOrCreatePSO(pso));
				m_commandList->SetPipelineState(pipeline->GetPipeline());

				RHI_PipelineLayout_DX12* pipelineLayout = static_cast<RHI_PipelineLayout_DX12*>(m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso));
				m_commandList->SetComputeRootSignature(pipelineLayout->GetRootSignature());

				{
					m_descriptorAllocator->SetPipeline(m_activeComputePSO.Shader);
#ifdef DX12_REUSE_DESCRIPTOR_TABLES
					m_boundDescriptorSets.clear();
					m_boundDescriptorSets.resize(m_descriptorAllocator->GetAllocatorDescriptorSets().size());
#endif // DX12_REUSE_DESCRIPTOR_TABLES
				}
			}

			void RHI_CommandList_DX12::BeginTimeBlock(const std::string& blockName)
			{
				BeginTimeBlock(blockName, Maths::Vector4(1, 1, 1, 1));
			}

			void RHI_CommandList_DX12::BeginTimeBlock(const std::string& blockName, Maths::Vector4 colour)
			{
				colour.x = std::max(0.0f, std::min(1.0f, colour.x));
				colour.y = std::max(0.0f, std::min(1.0f, colour.y));
				colour.z = std::max(0.0f, std::min(1.0f, colour.z));
				UINT pixColour = PIX_COLOR(static_cast<BYTE>(colour.x * 255), static_cast<BYTE>(colour.y * 255), static_cast<BYTE>(colour.z * 255));
				PIXBeginEvent(m_commandList, pixColour, blockName.c_str());
			}

			void RHI_CommandList_DX12::EndTimeBlock()
			{
				PIXEndEvent(m_commandList);
				m_nvtxRangehandle = {};
			}

			bool RHI_CommandList_DX12::BindDescriptorSets(const GPUQueue gpuQueue)
			{
				IS_PROFILE_FUNCTION();

				DescriptorHeapGPU_DX12& resouceHeap = m_contextDX12->GetFrameDescriptorHeapGPU();
				DescriptorHeapGPU_DX12& samplerHeap = m_contextDX12->GetFrameDescriptorHeapGPUSampler();

				if (m_boundResourceHeap == nullptr)
				{
					ID3D12DescriptorHeap* heaps[] =
					{
						resouceHeap.GetHeap(),
						samplerHeap.GetHeap(),
					};
					m_commandList->SetDescriptorHeaps(ARRAY_COUNT(heaps), heaps);
					m_boundResourceHeap = m_contextDX12->GetFrameDescriptorHeapGPU().GetHeap();
				}

				// While descriptor sets 'Set' value could be anything sets are arranged from 0 upwards.
				// Because of this the root parameter index they have could be different from the 'Set' value.
				// However the descriptor sets should be in the correct order so just increment 'rootParameterIdx'.
				u32 rootParameterIdx = 0;

				std::vector<DescriptorSet> const& descriptorSets = m_descriptorAllocator->GetAllocatorDescriptorSets();
				for (const auto& set : descriptorSets)
				{
					IS_PROFILE_SCOPE("Single Set");
					if (set.Bindings.size() == 0)
					{
						// If the set is a 'bindless' set then just continue.
						continue;
					}

					if (set.Bindings.size() == c_MaxRootDescriptorBindingForRootDescriptor
						&& (set.Bindings.at(0).Type == DescriptorType::Unifom_Buffer
							|| set.Bindings.at(0).Type == DescriptorType::Storage_Buffer))
					{
						// Root descriptor
						DescriptorBinding const& binding = set.Bindings.at(0);

						switch (set.Bindings.at(0).Type)
						{
						case DescriptorType::Unifom_Buffer:
						{
							ASSERT(binding.RHI_Buffer_View.size() <= 1);
							if (binding.RHI_Buffer_View.size() > 0)
							{
								const RHI_BufferView& buffer = binding.RHI_Buffer_View[0];
								if (buffer.IsValid())
								{
									IS_PROFILE_SCOPE("Root constant buffer");

									RHI::DX12::RHI_Buffer_DX12* bufferDX12 = static_cast<RHI::DX12::RHI_Buffer_DX12*>(buffer.GetBuffer());
									switch (gpuQueue)
									{
									case GPUQueue::GPUQueue_Graphics:
									{
										m_commandList->SetGraphicsRootConstantBufferView(rootParameterIdx,
											bufferDX12->GetResource()->GetGPUVirtualAddress() + buffer.GetOffset());
										break;
									}
									case GPUQueue::GPUQueue_Compute:
									{
										m_commandList->SetComputeRootConstantBufferView(rootParameterIdx,
											bufferDX12->GetResource()->GetGPUVirtualAddress() + buffer.GetOffset());
										break;
									}
									default:
										break;
									}
									++RenderStats::Instance().DescriptorSetBindings;
								}
							}

							break;
						}
						case DescriptorType::Storage_Buffer:
						{
							ASSERT(binding.RHI_Buffer_View.size() <= 1);
							if (binding.RHI_Buffer_View.size() > 0)
							{
								const RHI_BufferView& buffer = binding.RHI_Buffer_View[0];
								if (buffer.IsValid())
								{
									IS_PROFILE_SCOPE("Root UAV buffer");

									RHI::DX12::RHI_Buffer_DX12* bufferDX12 = static_cast<RHI::DX12::RHI_Buffer_DX12*>(buffer.GetBuffer());
									switch (gpuQueue)
									{
									case GPUQueue::GPUQueue_Graphics:
									{
										m_commandList->SetGraphicsRootUnorderedAccessView(rootParameterIdx,
											bufferDX12->GetResource()->GetGPUVirtualAddress() + buffer.GetOffset());
										break;
									}
									case GPUQueue::GPUQueue_Compute:
									{
										m_commandList->SetComputeRootUnorderedAccessView(rootParameterIdx,
											bufferDX12->GetResource()->GetGPUVirtualAddress() + buffer.GetOffset());
										break;
									}
									default:
										break;
									}
									++RenderStats::Instance().DescriptorSetBindings;
								}
							}
							break;
						}
						default:
							break;
						}
					}
					else
					{
						// Descriptor table
						DescriptorHeapHandle_DX12 firstHandle;

#ifdef DX12_REUSE_DESCRIPTOR_TABLES
						u64 const descriptorTableHash = set.DX_Hash;
						// Check if the data/values have already been assigned into the gpu descriptor heap.
						if (auto iter = m_descriptorTableCache.find(descriptorTableHash);
							iter != m_descriptorTableCache.end())
						{
							firstHandle = iter->second;
							set.Bindings.at(0).Type == DescriptorType::Sampler ?
							++RenderStats::Instance().DescriptorTableSamplerReuse : ++RenderStats::Instance().DescriptorTableResourceReuse;
						}
						else
#endif // DX12_REUSE_DESCRIPTOR_TABLES
						{
							set.Bindings.at(0).Type == DescriptorType::Sampler ? 
								++RenderStats::Instance().DescriptorTableSamplerCreations : ++RenderStats::Instance().DescriptorTableResourceCreations;

							for (auto const& binding : set.Bindings)
							{
								for (size_t bindingCountIdx = 0; bindingCountIdx < binding.Count; ++bindingCountIdx)
								{
									if ((binding.Type == DescriptorType::Unifom_Buffer
										|| binding.Type == DescriptorType::Storage_Buffer)
										&& bindingCountIdx < binding.RHI_Buffer_View.size())
									{
										const RHI_BufferView& buffer = binding.RHI_Buffer_View[bindingCountIdx];
										if (!buffer.IsValid())
										{
											continue;
										}

										DescriptorHeapHandle_DX12 dstHandle = resouceHeap.GetNextHandle();
										if (firstHandle.CPUPtr.ptr == 0)
										{
											firstHandle = dstHandle;
										}
										if (buffer.IsValid())
										{
											RHI::DX12::RHI_Buffer_DX12* bufferDX12 = static_cast<RHI::DX12::RHI_Buffer_DX12*>(buffer.GetBuffer());

											switch (binding.Type)
											{
											case DescriptorType::Unifom_Buffer:
											{
												IS_PROFILE_SCOPE("Create constant buffer");
												D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
												desc.BufferLocation = bufferDX12->GetResource()->GetGPUVirtualAddress() + buffer.GetOffset();
												desc.SizeInBytes = static_cast<UINT>(buffer.GetSize());
												m_contextDX12->GetDevice()->CreateConstantBufferView(&desc, dstHandle.CPUPtr);
												break;
											}
											case DescriptorType::Storage_Buffer:
											{
												IS_PROFILE_SCOPE("Create unordered access view buffer");
												D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
												desc.Format = DXGI_FORMAT_UNKNOWN;
												desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
												desc.Buffer.FirstElement = buffer.Vertex.UAVStartIndex;
												desc.Buffer.NumElements = static_cast<UINT>(buffer.Vertex.UAVNumOfElements);
												desc.Buffer.StructureByteStride = static_cast<UINT>(buffer.Vertex.Stride);

												m_contextDX12->GetDevice()->CreateUnorderedAccessView(bufferDX12->GetResource(), nullptr, &desc, dstHandle.CPUPtr);

												break;
											}
											default:
											{
												FAIL_ASSERT();
												break;
											}
											}
											++RenderStats::Instance().DescriptorSetUpdates;
										}
										else
										{
											IS_PROFILE_SCOPE("Copy null buffer");

											m_contextDX12->GetDevice()->CopyDescriptorsSimple(1, dstHandle.CPUPtr, m_contextDX12->GetDescriptorCBVNullHandle().CPUPtr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
											++RenderStats::Instance().DescriptorSetUpdates;
										}
									}

									if ((binding.Type == DescriptorType::Sampled_Image
										|| binding.Type == DescriptorType::Storage_Image)
										&& bindingCountIdx < binding.RHI_Texture.size())
									{
										DescriptorHeapHandle_DX12 dstHandle = resouceHeap.GetNextHandle();
										if (firstHandle.CPUPtr.ptr == 0)
										{
											firstHandle = dstHandle;
										}

										const RHI_Texture* texture = binding.RHI_Texture[bindingCountIdx];
										if (texture)
										{
											RHI_Texture_DX12 const* textureDX12 = static_cast<RHI_Texture_DX12 const*>(texture);
											if (textureDX12->GetResource())
											{
												IS_PROFILE_SCOPE("Copy texture");

												DescriptorHeapHandle_DX12 srvHandle = {};
												
												switch (binding.Type)
												{
												case DescriptorType::Sampled_Image:
												{
													srvHandle = textureDX12->GetDescriptorHandle();
													break;
												}
												case DescriptorType::Storage_Image:
												{
													srvHandle = textureDX12->GetUAVHandle();
													break;
												}
												default:
													break;
												}
												m_contextDX12->GetDevice()->CopyDescriptorsSimple(1, dstHandle.CPUPtr, srvHandle.CPUPtr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
												++RenderStats::Instance().DescriptorSetUpdates;
											}
										}
										else
										{
											IS_PROFILE_SCOPE("Copy null texture");

											//m_contextDX12->GetDevice()->CopyDescriptorsSimple(1, dstHandle.CPUPtr, m_contextDX12->GetDescriptorSRVNullHandle().CPUPtr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
											++RenderStats::Instance().DescriptorSetUpdates;
										}
									}

									if (binding.Type == DescriptorType::Sampler
										&& bindingCountIdx < binding.RHI_Sampler.size())
									{
										DescriptorHeapHandle_DX12 samplerHandle = samplerHeap.GetNextHandle();
										if (firstHandle.CPUPtr.ptr == 0)
										{
											firstHandle = samplerHandle;
										}

										const RHI_Sampler* sampler = binding.RHI_Sampler[bindingCountIdx];
										if (sampler)
										{
											IS_PROFILE_SCOPE("Copy sampler");

											RHI_Sampler_DX12 const* samplerDX12 = static_cast<RHI_Sampler_DX12 const*>(sampler);
											DescriptorHeapHandle_DX12 srvHandle = samplerDX12->Handle;
											m_contextDX12->GetDevice()->CopyDescriptorsSimple(1, samplerHandle.CPUPtr, srvHandle.CPUPtr, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
											++RenderStats::Instance().DescriptorSetUpdates;
										}
										else
										{
											IS_PROFILE_SCOPE("Copy null sampler");

											//m_contextDX12->GetDevice()->CopyDescriptorsSimple(1, samplerHandle.CPUPtr, m_contextDX12->GetDescriptorSAMNullHandle().CPUPtr, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
											++RenderStats::Instance().DescriptorSetUpdates;
										}
									}
								}
							}
#ifdef DX12_REUSE_DESCRIPTOR_TABLES
							m_descriptorTableCache[descriptorTableHash] = firstHandle;
#endif // DX12_REUSE_DESCRIPTOR_TABLES
						}

						if (firstHandle.GPUPtr.ptr != 0 
#ifdef DX12_REUSE_DESCRIPTOR_TABLES
							&& m_boundDescriptorSets.at(rootParameterIdx).GPUPtr.ptr != firstHandle.GPUPtr.ptr
#endif // DX12_REUSE_DESCRIPTOR_TABLES
							)
						{
							IS_PROFILE_SCOPE("Set table");

							m_boundDescriptorSets.at(rootParameterIdx) = firstHandle;
							switch (gpuQueue)
							{
							case GPUQueue::GPUQueue_Graphics:
							{
								m_commandList->SetGraphicsRootDescriptorTable(rootParameterIdx, firstHandle.GPUPtr);
								break;
							}
							case GPUQueue::GPUQueue_Compute:
							{
								m_commandList->SetComputeRootDescriptorTable(rootParameterIdx, firstHandle.GPUPtr);
								break;
							}
							default:
								break;
							}
							++RenderStats::Instance().DescriptorSetBindings;
						}
					}
					++rootParameterIdx;
				}
				
				return true;
			}

			void RHI_CommandList_DX12::SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout)
			{
				ImageLayout image_layout = texture->GetLayout();
				Graphics::PipelineBarrier pipeline_barrier = { };

				ImageBarrier image_barrer = {};
				image_barrer.SrcAccessFlags = ImageLayoutToAccessMask(image_layout);
				image_barrer.DstAccessFlags = ImageLayoutToAccessMask(layout);
				image_barrer.OldLayout = image_layout;
				image_barrer.NewLayout = layout;
				image_barrer.Image = texture;
				image_barrer.SubresourceRange = ImageSubresourceRange::SingleMipAndLayer(PixelFormatToAspectFlags(texture->GetFormat()));

				if (image_layout == ImageLayout::PresentSrc)
				{
					pipeline_barrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::BottomOfPipe);
				}
				else if (image_layout == ImageLayout::Undefined)
				{
					pipeline_barrier.SrcStage = static_cast<u32>(PipelineStageFlagBits::TopOfPipe);
				}
				else
				{
					pipeline_barrier.SrcStage = AccessFlagBitsToPipelineStageFlag(image_barrer.SrcAccessFlags);
				}

				if (image_layout == ImageLayout::PresentSrc)
				{
					pipeline_barrier.DstStage = static_cast<u32>(PipelineStageFlagBits::TopOfPipe);
				}
				else
				{
					pipeline_barrier.DstStage = AccessFlagBitsToPipelineStageFlag(image_barrer.DstAccessFlags);
				}

				pipeline_barrier.ImageBarriers.push_back(image_barrer);
				PipelineBarrier(pipeline_barrier);
			}

			ID3D12CommandAllocator* RHI_CommandListAllocator_DX12::GetAllocator() const
			{
				return m_allocator.Get();
			}

			//// <summary>
			//// RHI_CommandListAllocator_DX12
			//// </summary>
			//// <param name="context"></param>
			void RHI_CommandListAllocator_DX12::Create(RenderContext* context, const RHI_CommandListAllocatorDesc desc)
			{
				{
					std::lock_guard lock(m_mutex);
					m_context = static_cast<RenderContext_DX12*>(context);
					ThrowIfFailed(m_context->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)));
				}

				std::vector<RHI_CommandList*> cmdLists;
				for (size_t i = 0; i < desc.CommandListSize; ++i)
				{
					cmdLists.push_back(GetCommandList());
				}
				for (size_t i = 0; i < cmdLists.size(); ++i)
				{
					cmdLists.at(i)->Close();
				}
				Reset();
			}

			RHI_CommandList* RHI_CommandListAllocator_DX12::GetCommandList()
			{
				std::lock_guard lock(m_mutex);
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
				list->SetName("CmdList_" + std::to_string(m_allocLists.size() + m_freeLists.size()));

				m_allocLists.insert(list);
				return list;
			}

			void RHI_CommandListAllocator_DX12::Reset()
			{
				std::lock_guard lock(m_mutex);
				m_allocator->Reset();
				while (m_allocLists.size() > 0)
				{
					RHI_CommandList* list = *m_allocLists.begin();
					//list->Reset();

					m_freeLists.insert(list);
					m_allocLists.erase(list);
				}
			}

			void RHI_CommandListAllocator_DX12::Release()
			{
				if (m_allocator)
				{
					Reset();

					std::lock_guard lock(m_mutex);
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
				std::lock_guard lock(m_mutex);
				return m_allocator;
			}

			void RHI_CommandListAllocator_DX12::SetName(std::string name)
			{
				std::lock_guard lock(m_mutex);
				if (m_allocator)
				{
					m_context->SetObjectName(name, m_allocator.Get());
				}
				m_name = name;
			}
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)