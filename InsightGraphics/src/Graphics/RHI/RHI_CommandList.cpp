#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RenderContext.h"

#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/DX12/RHI_CommandList_DX12.h"
#include "Graphics/RHI/Vulkan/RHI_CommandList_Vulkan.h"

#include "Tracy.hpp"

namespace Insight
{
	namespace Graphics
	{
		/// <summary>
		/// RHI_CommandList
		/// </summary>
		/// <returns></returns>
		RHI_CommandList* RHI_CommandList::New()
		{
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_CommandList_Vulkan); }
			if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_CommandList_DX12); }
			return nullptr;
		}

		void RHI_CommandList::Record(CommandList& cmdList, FrameResouce* frameResouces)
		{
			m_frameResouces = frameResouces;

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
					SetPipeline(m_pso);
					break;
				}

				case CommandType::SetUniform:
				{
					ZoneScopedN("SetUniform");
					const CMD_SetUniform* cmd = dynamic_cast<const CMD_SetUniform*>(command);
					SetUniform(cmd->Set, cmd->Binding, cmd->View);
					break;
				}

				case CommandType::SetViewport:
				{
					ZoneScopedN("SetViewport");
					const CMD_SetViewport* cmd = dynamic_cast<const CMD_SetViewport*>(command);
					SetViewport(0.0f, 0.0f, (float)cmd->Width, (float)cmd->Height, 0.0f, 1.0f);
					break;
				}

				case CommandType::SetScissor:
				{
					ZoneScopedN("SetScissor");
					const CMD_SetScissor* cmd = dynamic_cast<const CMD_SetScissor*>(command);
					SetScissor(0, 0, cmd->Width, cmd->Height);
					break;
				}

				case CommandType::SetVertexBuffer:
				{
					const CMD_SetVertexBuffer* cmd = dynamic_cast<const CMD_SetVertexBuffer*>(command);
					SetVertexBuffer(cmd->Buffer);
					break;
				}

				case CommandType::SetIndexBuffer:
				{
					const CMD_SetIndexBuffer* cmd = dynamic_cast<const CMD_SetIndexBuffer*>(command);
					SetIndexBuffer(cmd->Buffer);
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
					Draw(cmd->VertexCount, cmd->InstanceCount, cmd->FirstVertex, cmd->FirstInstance);
					break;
				}

				case CommandType::DrawIndexed:
				{
					if (!CanDraw(cmdList))
					{
						break;
					}
					const CMD_DrawIndexed* cmd = dynamic_cast<const CMD_DrawIndexed*>(command);
					DrawIndexed(cmd->IndexCount, cmd->InstanceCount, cmd->FirstIndex, cmd->VertexOffset, cmd->FirstInstance);
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

		void RHI_CommandList::Reset()
		{
			m_pso = {};
			m_activePSO = {};
		}

		bool RHI_CommandList::CanDraw(CommandList& cmdList)
		{
			if (m_pso.GetHash() != m_activePSO.GetHash())
			{
				m_activePSO = m_pso;
				RHI_DescriptorLayout* layout = m_context->GetDescriptorLayoutManager().GetLayout(0, cmdList.GetDescriptorBuffer().GetDescriptors());
				BindPipeline(m_activePSO, layout);
			}
			return BindDescriptorSets();
		}

		bool RHI_CommandList::BindDescriptorSets()
		{
			return false;
		}


		/// <summary>
		/// RHI_CommandListAllocator
		/// </summary>
		/// <returns></returns>
		RHI_CommandListAllocator* RHI_CommandListAllocator::New()
		{
			if (GraphicsManager::IsVulkan()) { return NewTracked(RHI::Vulkan::RHI_CommandListAllocator_Vulkan); }
			if (GraphicsManager::IsDX12()) { return NewTracked(RHI::DX12::RHI_CommandListAllocator_DX12); }
			return nullptr;
		}

		void RHI_CommandListAllocator::ReturnCommandList(RHI_CommandList* cmdList)
		{
			//if (m_allocLists.find(cmdList) == m_allocLists.end())
			//{
			//	IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is not in the allocated list. Command lists should be obtained by 'GetCommandList'.");
			//}
			//m_allocLists.erase(cmdList);
			//
			//if (m_freeLists.find(cmdList) != m_freeLists.end())
			//{
			//	IS_CORE_ERROR("[RHI_CommandListAllocator::ReturnCommandList] CommandList is in the free list. Command should not be returned more than once.");
			//	return;
			//}
			//m_freeLists.insert(cmdList);
		}

		/// <summary>
		/// CommandListManager
		/// </summary>
		CommandListManager::CommandListManager()
		{
		}

		CommandListManager::~CommandListManager()
		{
		}

		void CommandListManager::Create(RenderContext* context)
		{
			m_context = context;

			if (m_allocator)
			{
				IS_CORE_WARN("[CommandListManager::Init] CommandListManager already has 'Init' called.");
				return;
			}

			m_allocator = RHI_CommandListAllocator::New();
			m_allocator->Create(m_context);
		}

		void CommandListManager::Update()
		{
			m_allocator->Reset();
		}

		void CommandListManager::Destroy()
		{
			if (m_allocator)
			{
				m_allocator->Release();
				DeleteTracked(m_allocator);
			}
		}

		RHI_CommandList* CommandListManager::GetCommandList()
		{
			assert(m_allocator);
			return m_allocator->GetCommandList();
		}

		RHI_CommandList* CommandListManager::GetSingleUseCommandList()
		{
			assert(m_allocator);
			return m_allocator->GetSingleSubmitCommandList();
		}

		void CommandListManager::ReturnCommandList(RHI_CommandList* cmdList)
		{
			assert(m_allocator);
			m_allocator->ReturnCommandList(cmdList);
		}

		void CommandListManager::ReturnSingleUseCommandList(RHI_CommandList* cmdList)
		{
			assert(m_allocator);
			m_allocator->ReturnSingleSubmitCommandList(cmdList);
		}
	}
}