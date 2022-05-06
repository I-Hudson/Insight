#include "Graphics/RHI/DX12/CommandList_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_Buffer_DX12.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			CommandList_DX12::CommandList_DX12()
			{
			}

			CommandList_DX12::CommandList_DX12(RenderContext_DX12* context)
			{
				m_context = context;
			}

			CommandList_DX12::~CommandList_DX12()
			{
			}

			void CommandList_DX12::Record(CommandList& cmdList, FrameResourceDX12& frameResouces)
			{
				ZoneScoped;
				m_frameResouces = &frameResouces;
				m_frameResouces->UniformBuffer.Reset();
				m_frameResouces->UniformBuffer.Resize(cmdList.GetDescriptorBuffer().GetCapacity());

				m_frameResouces->UniformBuffer.Upload(cmdList.GetDescriptorBuffer().GetData(), cmdList.GetDescriptorBuffer().GetSize());

				for (int i = 0; i < cmdList.GetCommandCount(); ++i)
				{
					ZoneScopedN("Single Command");
					ICommand* command = cmdList.GetCurrentCommand();
					switch (command->CommandType)
					{
					case CommandType::None:

					case CommandType::SetPipelineStateObject:
					{
						ZoneScopedN("SetPipelineStateObject");
						const CMD_SetPipelineStateObject* cmd = dynamic_cast<const CMD_SetPipelineStateObject*>(command);
						m_pso = cmd->Pso;
						m_frameResouces->DescriptorAllocator.SetPipeline(m_pso);
						break;
					}

					case CommandType::SetUniform:
					{
						ZoneScopedN("SetUniform");
						const CMD_SetUniform* cmd = dynamic_cast<const CMD_SetUniform*>(command);
						m_frameResouces->DescriptorAllocator.SetUniform(cmd->Set, cmd->Binding, m_frameResouces->UniformBuffer.GetView(cmd->View.Offset, cmd->View.SizeInBytes));
						break;
					}

					case CommandType::SetViewport:
					{
						ZoneScopedN("SetViewport");
						const CMD_SetViewport* cmd = dynamic_cast<const CMD_SetViewport*>(command);
						std::array<D3D12_VIEWPORT, 1> viewports = { D3D12_VIEWPORT{ 0.0f, 0.0f, (float)cmd->Width, (float)cmd->Height, 0.0f, 1.0f } };
						m_commandList->RSSetViewports(1, viewports.data());
						break;
					}

					case CommandType::SetScissor:
					{
						ZoneScopedN("SetScissor");
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
						ZoneScopedN("Draw");
						if (!CanDraw(cmdList))
						{
							break;
						}
						const CMD_Draw* cmd = dynamic_cast<const CMD_Draw*>(command);
						m_commandList->DrawInstanced(cmd->VertexCount, cmd->InstanceCount, cmd->FirstVertex, cmd->FirstInstance);
						break;
					}

					case CommandType::DrawIndexed:
					{
						if (!CanDraw(cmdList))
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
				m_frameResouces = nullptr;
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

			void CommandList_DX12::Release()
			{
				if (m_commandList)
				{
					m_commandList.Reset();
				}
			}

			bool CommandList_DX12::CanDraw(CommandList& cmdList)
			{
				ZoneScoped;
				ID3D12PipelineState* pipeline = m_context->GetPipelineStateObjectManager().GetOrCreatePSO(m_pso);
				if (m_pso.GetHash() != m_activePSO.GetHash())
				{
					m_activePSO = m_pso;
					m_commandList->SetPipelineState(pipeline);

					RHI_DescriptorLayout_DX12* layout = dynamic_cast<RHI_DescriptorLayout_DX12*>(m_context->GetDescriptorLayoutManager().GetLayout(0, cmdList.GetDescriptorBuffer().GetDescriptors()));
					m_commandList->SetGraphicsRootSignature(layout->GetRootSignature());
					m_commandList->IASetPrimitiveTopology(PrimitiveTopologyTypeToDX12(m_activePSO.PrimitiveTopologyType));
				}

				return BindDescriptorSets();
			}

			bool CommandList_DX12::BindDescriptorSets()
			{
				ZoneScoped;
				bool result = true;// m_frameResouces->DescriptorAllocator.SetupDescriptors();
				m_frameResouces->DescriptorAllocator.BindTempConstentBuffer(this, m_frameResouces->DescriptorAllocator.GetDescriptor(0,0).BufferView, 0);
				std::vector<ID3D12DescriptorHeap*> descriptors = m_frameResouces->DescriptorAllocator.GetHeaps();
				if (result && descriptors.size() > 0)
				{
					// Set our descriptor heaps.
					m_commandList->SetDescriptorHeaps(static_cast<UINT>(descriptors.size()), descriptors.data());

					// Set all our descriptors tables.
					//m_frameResouces->DescriptorAllocator.SetDescriptors(this);

				}
				return true;
			}



			CommandAllocator_DX12::CommandAllocator_DX12()
			{
				m_allocLists = {};
				m_freeLists = {};
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
					m_allocLists[i]->Reset();
					m_allocLists[i]->m_commandList->Reset(m_allocator.Get(), nullptr);
					m_freeLists.push_back(m_allocLists[i]);
				}
				m_allocLists.clear();
			}

			void CommandAllocator_DX12::Destroy()
			{
				for (auto& list : m_allocLists)
				{
					list->Release();
					DeleteTracked(list);
				}
				for (auto& list : m_freeLists)
				{
					list->Release();
					DeleteTracked(list);
				}

				m_allocLists.resize(0);
				m_freeLists.resize(0);
				m_allocator.Reset();
			}

			CommandList_DX12* CommandAllocator_DX12::GetCommandList()
			{
				if (m_freeLists.size() > 0)
				{
					CommandList_DX12* list = m_freeLists.back();
					m_freeLists.pop_back();
					m_allocLists.push_back(list);
					return m_allocLists.back();
				}

				CommandList_DX12* list = NewArgsTracked(CommandList_DX12, m_context);
				ThrowIfFailed(m_context->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr, IID_PPV_ARGS(& list->GetCommandBufferInteral())));

				m_allocLists.push_back(list);
				return m_allocLists.back();
			}

			void CommandAllocator_DX12::ReturnCommandList(CommandList_DX12& cmdList)
			{
			}
		}
	}
}