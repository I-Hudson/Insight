#include "Graphics/RHI/DX12/CommandList_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{

			CommandList_DX12::CommandList_DX12(RenderContext_DX12* context, ComPtr<ID3D12GraphicsCommandList> cmdList)
			{
				m_commandList = cmdList;
				m_context = context;
			}

			void CommandList_DX12::Record(CommandList& cmdList)
			{
				for (int i = 0; i < cmdList.GetCommandCount(); ++i)
				{
					ICommand* command = cmdList.GetCurrentCommand();
					switch (command->CommandType)
					{
					case CommandType::None:

					case CommandType::SetPipelineStateObject:
					{
						const CMD_SetPipelineStateObject* cmd = dynamic_cast<const CMD_SetPipelineStateObject*>(command);
						m_pso = cmd->Pso;
						break;
					}

					case CommandType::SetUniformBuffer:
					{
						const CMD_SetDescriptorBuffer* cmd = dynamic_cast<const CMD_SetDescriptorBuffer*>(command);
						m_activeDescriptorBuffer = cmd->Buffer;
						break;
					}

					case CommandType::SetViewport:
					{
						const CMD_SetViewport* cmd = dynamic_cast<const CMD_SetViewport*>(command);
						std::array<D3D12_VIEWPORT, 1> viewports = { D3D12_VIEWPORT{ 0.0f, 0.0f, (float)cmd->Width, (float)cmd->Height, 0.0f, 1.0f } };
						m_commandList->RSSetViewports(1, viewports.data());
						break;
					}

					case CommandType::SetScissor:
					{
						const CMD_SetScissor* cmd = dynamic_cast<const CMD_SetScissor*>(command);
						std::array<D3D12_RECT, 1> scissors = { D3D12_RECT{ 0, 0, cmd->Width, cmd->Height } };
						m_commandList->RSSetScissorRects(1, scissors.data());
						break;
					}

					case CommandType::SetVertexBuffer:
					{
						const CMD_SetVertexBuffer* cmd = dynamic_cast<const CMD_SetVertexBuffer*>(command);
						//std::array<vk::Buffer, 1> buffers = { cmd-> };
						//m_commandBuffer.bindVertexBuffers(0, buffers, offset);
						break;
					}

					case CommandType::Draw:
					{
						if (!CanDraw())
						{
							break;
						}
						const CMD_Draw* cmd = dynamic_cast<const CMD_Draw*>(command);
						m_commandList->DrawInstanced(cmd->VertexCount, cmd->InstanceCount, cmd->FirstVertex, cmd->FirstInstance);
						break;
					}

					case CommandType::DrawIndexed:
					{
						if (!CanDraw())
						{
							break;
						}
						const CMD_DrawIndexed* cmd = dynamic_cast<const CMD_DrawIndexed*>(command);
						//m_commandBuffer.drawIndexed(cmd->IndexCount, cmd->InstanceCount, cmd->FirstIndex, cmd->VertexOffset, cmd->FirstInstance);
						break;
					}

					default:
					{
						IS_CORE_ERROR("[CommandList_DX12::Record] Unknown command.");
						break;
					}
					}
					cmdList.NextCommand();
				}

				if (m_activeRenderpass)
				{
					m_activeRenderpass = false;
				}
			}

			void CommandList_DX12::Reset()
			{
				m_pso = {};
				m_activePSO = {};
			}

			void CommandList_DX12::Close()
			{
				ThrowIfFailed(m_commandList->Close());
			}

			bool CommandList_DX12::CanDraw()
			{
				ID3D12PipelineState* pipeline = m_context->GetPipelineStateObjectManager().GetOrCreatePSO(m_pso);
				if (m_pso.GetHash() != m_activePSO.GetHash())
				{
					m_activePSO = m_pso;
					m_commandList->SetPipelineState(pipeline);
					m_commandList->SetGraphicsRootSignature(m_context->GetPipelineStateObjectManager().GetRootSignature(m_activePSO));
					m_commandList->IASetPrimitiveTopology(PrimitiveTopologyTypeToDX12(m_activePSO.PrimitiveTopologyType));
				}

				m_context->GetDescriptorManager().GetDescriptor(m_activeDescriptorBuffer);
				return true;
			}



			void CommandAllocator_DX12::Init(RenderContext_DX12* context)
			{
				if (m_allocator)
				{
					return;
				}
				m_context = context;
				ThrowIfFailed(m_context->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator)));
			}

			void CommandAllocator_DX12::Update()
			{
				m_allocator->Reset();

				for (size_t i = 0; i < m_allocLists.size(); ++i)
				{
					m_allocLists[i].Reset();
					m_allocLists[i].m_commandList->Reset(m_allocator.Get(), nullptr);
					m_freeLists.push_back(m_allocLists[i]);
				}
				m_allocLists.clear();
			}

			void CommandAllocator_DX12::Destroy()
			{
				m_allocLists.resize(0);
				m_freeLists.resize(0);
				m_allocator.Reset();
			}

			CommandList_DX12& CommandAllocator_DX12::GetCommandList()
			{
				if (m_freeLists.size() > 0)
				{
					CommandList_DX12 list = m_freeLists.back();
					m_freeLists.pop_back();
					m_allocLists.push_back(list);
					return m_allocLists.back();
				}

				ComPtr<ID3D12GraphicsCommandList> commandList;
				ThrowIfFailed(m_context->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

				CommandList_DX12 list = CommandList_DX12(m_context, commandList);
				m_allocLists.push_back(list);
				return m_allocLists.back();
			}

			void CommandAllocator_DX12::ReturnCommandList(CommandList_DX12& cmdList)
			{
			}
		}
	}
}