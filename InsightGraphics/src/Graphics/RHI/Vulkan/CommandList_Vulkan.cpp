#include "Graphics/RHI/Vulkan/CommandList_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			CommandList_Vulkan::CommandList_Vulkan(RenderContext_Vulkan* context, vk::CommandBuffer cmdBuffer)
			{
				m_commandBuffer = cmdBuffer;
				m_context = context;
			}

			void CommandList_Vulkan::Record(CommandList& cmdList)
			{
				vk::CommandBufferBeginInfo info = vk::CommandBufferBeginInfo();
				m_commandBuffer.begin(info);

				for (int  i = 0; i < cmdList.GetCommandCount(); ++i)
				{
					ICommand* command = cmdList.GetCurrentCommand();
					switch (command->CommandType)
					{
					case CommandType::None:

					case CommandType::SetPipelineStateObject:
					{
						const CMD_SetPipelineStateObject* cmd = dynamic_cast<const CMD_SetPipelineStateObject*>(command);
						m_pso = cmd->Pso;;
						break;
					}

					case CommandType::SetViewport:
					{
						const CMD_SetViewport* cmd = dynamic_cast<const CMD_SetViewport*>(command);
						std::array<vk::Viewport, 1> viewports = { vk::Viewport(0, 0, cmd->Width, cmd->Height) };
						m_commandBuffer.setViewport(0, viewports);
						break;
					}

					case CommandType::SetScissor:
					{
						const CMD_SetScissor* cmd = dynamic_cast<const CMD_SetScissor*>(command);
						std::array<vk::Rect2D, 1> scissors = { vk::Rect2D(vk::Offset2D(0, 0), 
							vk::Extent2D(static_cast<uint32_t>(cmd->Width), static_cast<uint32_t>(cmd->Height))) };
						m_commandBuffer.setScissor(0, scissors);
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
						m_commandBuffer.draw(cmd->VertexCount, cmd->InstanceCount, cmd->FirstVertex, cmd->FirstInstance);
						break;
					}

					case CommandType::DrawIndexed:
					{
						if (!CanDraw())
						{
							break;
						}
						const CMD_DrawIndexed* cmd = dynamic_cast<const CMD_DrawIndexed*>(command);
						m_commandBuffer.drawIndexed(cmd->IndexCount, cmd->InstanceCount, cmd->FirstIndex, cmd->VertexOffset, cmd->FirstInstance);
						break;
					}

					default:
						break;
					}
					cmdList.NextCommand();
				}

				m_commandBuffer.end();
			}

			bool CommandList_Vulkan::CanDraw()
			{
				vk::Pipeline pipeline = m_context->GetPipelineStateObjectManager().GetOrCreatePSO(m_pso);
				if (m_pso.GetHash() != m_activePSO.GetHash())
				{
					m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
				}

				return true;
			}


			void CommandPool_Vulkan::Init(RenderContext_Vulkan* context)
			{
				if (m_pool)
				{
					IS_CORE_ERROR("[CommandPool_Vulkan::Init] Command pool already setup.");
					return;
				}
				m_context = context;

				vk::CommandPoolCreateInfo info = vk::CommandPoolCreateInfo();
				m_pool = m_context->GetDevice().createCommandPool(info);
			}

			void CommandPool_Vulkan::Update()
			{
				for (size_t i = 0; i < m_allocLists.size(); ++i)
				{
					m_freeLists.push_back(m_allocLists[i]);
				}
				m_allocLists.clear();
				m_context->GetDevice().resetCommandPool(m_pool);
			}

			void CommandPool_Vulkan::Destroy()
			{
				m_context->GetDevice().resetCommandPool(m_pool);
				m_context->GetDevice().destroyCommandPool(m_pool);
			}

			CommandList_Vulkan& CommandPool_Vulkan::GetCommandList()
			{
				if (m_freeLists.size() > 0)
				{
					CommandList_Vulkan& list = m_freeLists.back();
					m_freeLists.pop_back();
					m_allocLists.push_back(list);
					return m_allocLists.back();
				}

				vk::CommandBufferAllocateInfo info = vk::CommandBufferAllocateInfo(m_pool);
				info.setCommandBufferCount(1);
				CommandList_Vulkan list = CommandList_Vulkan(m_context, m_context->GetDevice().allocateCommandBuffers(info)[0]);
				m_allocLists.push_back(list);
				return m_allocLists.back();
			}

			void CommandPool_Vulkan::ReturnCommandList(CommandList_Vulkan& cmdList)
			{
				std::vector<CommandList_Vulkan>::iterator itr = std::find(m_freeLists.begin(), m_freeLists.end(), cmdList);
				if (itr == m_freeLists.end())
				{
					m_freeLists.push_back(cmdList);
				}
				else
				{
					IS_CORE_ERROR("[CommandPool_Vulkan::ReturnCommandList] CommandList is already free.");
				}

				itr = std::find(m_allocLists.begin(), m_allocLists.end(), cmdList);
				if (itr != m_allocLists.end())
				{
					m_allocLists.erase(itr);
				}
				else
				{
					IS_CORE_ERROR("[CommandPool_Vulkan::ReturnCommandList] CommandList not allocated.");
				}
			}
		}
	}
}
