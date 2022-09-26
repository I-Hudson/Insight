#include "Graphics/GPU/RHI/DX12/GPUCommandList_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUSemaphore_DX12.h"
#include "Graphics/GPU/RHI/DX12/GPUFence_DX12.h"
#include "Graphics/GPU/RHI/DX12/DX12Utils.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			GPUCommandList_DX12::GPUCommandList_DX12()
			{
			}

			GPUCommandList_DX12::~GPUCommandList_DX12()
			{
			}

			void GPUCommandList_DX12::CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, u64 srcOffset, u64 dstOffset, u64 size)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::CopyBufferToBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::CopyBufferToBuffer] CommandList is not recording."); return; }

				m_commandList->CopyBufferRegion(nullptr, dstOffset, nullptr, srcOffset, size);
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::SetViewport(int width, int height)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::SetViewport] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::SetViewport] CommandList is not recording."); return; }

				D3D12_VIEWPORT viewports[1] = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
				m_commandList->RSSetViewports(1, viewports);
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::SetScissor(int width, int height)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::SetScissor] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::SetScissor] CommandList is not recording."); return; }

				D3D12_RECT rects[1] = { static_cast<LONG>(0), static_cast<LONG>(0), static_cast<LONG>(width), static_cast<LONG>(height) };
				m_commandList->RSSetScissorRects(1, rects);
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::SetVertexBuffer(GPUBuffer* buffer)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::SetVertexBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::SetVertexBuffer] CommandList is not recording."); return; }

				if (!buffer || m_activeItems.VertexBuffer == buffer)
				{
					return;
				}

				D3D12_VERTEX_BUFFER_VIEW buffers[1] = { {} };
				m_commandList->IASetVertexBuffers(0, 1, buffers);
				++m_recordCommandCount;
				m_activeItems.VertexBuffer = buffer;
			}

			void GPUCommandList_DX12::SetIndexBuffer(GPUBuffer* buffer)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::SetIndexBuffer] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::SetIndexBuffer] CommandList is not recording."); return; }

				if (!buffer || m_activeItems.IndexBuffer == buffer)
				{
					return;
				}

				D3D12_INDEX_BUFFER_VIEW iBuffer = { {} };
				m_commandList->IASetIndexBuffer(&iBuffer);
				++m_recordCommandCount;
				m_activeItems.IndexBuffer = buffer;
			}

			void GPUCommandList_DX12::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::Draw] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::Draw] CommandList is not recording."); return; }

				m_commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::DrawIndexed] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::DrawIndexed] CommandList is not recording."); return; }

				m_commandList->DrawIndexedInstanced(indexCount, instanceCount,firstIndex, vertexOffset, firstInstance);
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::Submit] CommandList is null."); return; }
				if (m_recordCommandCount == 0) { IS_CORE_ERROR("[GPUCommandList_DX12::Submit] Record command count is 0. Nothing is submitted."); return; }

				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				m_commandList->Close();
				ID3D12CommandList* cmdLists[1] = { m_commandList.Get() };
				ID3D12CommandQueue* cmdQueue = GetDevice()->GetQueue(queue);

				for (const GPUSemaphore* smeaphore : waitSemaphores)
				{
					const GPUSemaphore_DX12* semaphoreDX12 = dynamic_cast<const GPUSemaphore_DX12*>(smeaphore);
					ThrowIfFailed(cmdQueue->Wait(semaphoreDX12->GetSemaphore(), c_DX12SemaphoreSignal));
				}

				cmdQueue->ExecuteCommandLists(1, cmdLists);

				for (const GPUSemaphore* smeaphore : signalSemaphores)
				{
					const GPUSemaphore_DX12* semaphoreDX12 = dynamic_cast<const GPUSemaphore_DX12*>(smeaphore);
					ThrowIfFailed(cmdQueue->Signal(semaphoreDX12->GetSemaphore(), c_DX12SemaphoreSignal));
				}
				//if (fence)
				//{
				//	fence->Reset();
				//}
			}

			void GPUCommandList_DX12::BeginRecord()
			{
				Reset();
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRecord] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Idle) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRecord] CommandList is already recording."); return; }
				m_state = GPUCommandListState::Recording;

				ThrowIfFailed(m_commandList->Reset(m_allocator->GetCommandPool(m_type), nullptr));
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::EndRecord()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::EndRecord] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::EndRecord] CommandList is not recording."); return; }

				if (m_activeItems.Renderpass)
				{
					EndRenderpass();
				}

				m_state = GPUCommandListState::Idle;
				//m_commandList->Close();
				++m_recordCommandCount;
			}

			void GPUCommandList_DX12::BeginRenderpass()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRenderpass] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRenderpass] CommandList is not recording."); return; }
				if (m_activeItems.Renderpass) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRenderpass] Renderpass must call End before Begin can be called."); return; }

			}

			void GPUCommandList_DX12::EndRenderpass()
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::EndRenderpass] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::EndRenderpass] CommandList is not recording."); return; }
				if (!m_activeItems.Renderpass) { IS_CORE_ERROR("[GPUCommandList_DX12::BeginRenderpass] Renderpass must call Begin before End can be called."); return; }


			}

			void GPUCommandList_DX12::BindPipeline(GPUPipelineStateObject* pipeline)
			{
				if (!m_commandList) { IS_CORE_ERROR("[GPUCommandList_DX12::BindPipeline] CommandList is null."); return; }
				if (m_state != GPUCommandListState::Recording) { IS_CORE_ERROR("[GPUCommandList_DX12::BindPipeline] CommandList is not recording."); return; }
			}


			GPUComamndListAllocator_DX12::GPUComamndListAllocator_DX12()
			{
			}

			GPUComamndListAllocator_DX12::~GPUComamndListAllocator_DX12()
			{
				FreeAllCommandLists();
				Destroy();
			}

			GPUCommandList* GPUComamndListAllocator_DX12::AllocateCommandList(GPUCommandListType type)
			{
				ComPtr<ID3D12CommandAllocator>& commandPool = m_commandPools[type];
				if (!commandPool)
				{
					ThrowIfFailed(GetDevice()->GetDevice()->CreateCommandAllocator(GPUCommandListTypeToCommandListTypeDX12(type), IID_PPV_ARGS(&commandPool)));
				}

				ID3D12GraphicsCommandList* commandList;
				ThrowIfFailed(GetDevice()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandPool.Get(), nullptr, IID_PPV_ARGS(&commandList)));
				commandList->Close();

				GPUCommandList_DX12* cmdList = new GPUCommandList_DX12();
				cmdList->m_commandList = commandList;
				cmdList->m_queue = m_queue;
				cmdList->m_type = type;
				cmdList->m_allocator = this;
				m_allocatedCommandLists.push_back(cmdList);

				return cmdList;
			}

			void GPUComamndListAllocator_DX12::ResetCommandLists(std::list<GPUCommandList*> cmdLists)
			{
				// TOOD:
				IS_CORE_ERROR("[GPUComamndListAllocator_DX12::ResetCommandPool] DX12 does not support this.");
			}

			void GPUComamndListAllocator_DX12::ResetCommandPool(GPUCommandListType type)
			{
				ComPtr<ID3D12CommandAllocator>& cmdPool = m_commandPools[type];
				if (cmdPool)
				{
					ThrowIfFailed(cmdPool->Reset());
				}
			}

			void GPUComamndListAllocator_DX12::FreeCommandList(GPUCommandList* cmdList)
			{
				FreeCommandLists({ cmdList });
			}

			void GPUComamndListAllocator_DX12::FreeCommandLists(const std::list<GPUCommandList*>& cmdLists)
			{
				if (m_allocatedCommandLists.size() == 0)
				{
					return;
				}
				for (const std::list<GPUCommandList*>::iterator::value_type& ptr : cmdLists)
				{
					std::list<GPUCommandList*>::iterator cmdListItr = std::find(m_allocatedCommandLists.begin(), m_allocatedCommandLists.end(), ptr);
					if (cmdListItr == m_allocatedCommandLists.end())
					{
						continue;
					}
					GPUCommandList_DX12* cmdListDX12 = dynamic_cast<GPUCommandList_DX12*>(*cmdListItr);
					cmdListDX12->m_commandList.Reset();
					delete cmdListDX12;
					m_allocatedCommandLists.erase(cmdListItr);
				}
			}

			void GPUComamndListAllocator_DX12::FreeAllCommandLists()
			{
				std::list<GPUCommandList*> list = m_allocatedCommandLists;
				FreeCommandLists(list);
			}

			ID3D12CommandAllocator* GPUComamndListAllocator_DX12::GetCommandPool(GPUCommandListType type) const
			{
				return m_commandPools.at(type).Get();
			}

			void GPUComamndListAllocator_DX12::Destroy()
			{
				for (auto& pair : m_commandPools)
				{
					if (pair.second)
					{
						pair.second->Reset();
						pair.second = nullptr;
					}
				}
				m_commandPools.clear();
			}
		}
	}
}