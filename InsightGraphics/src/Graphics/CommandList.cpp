#include "Graphics/CommandList.h"
#include "Core/MemoryTracker.h"
#include "Core/Memory.h"

namespace Insight
{
	namespace Graphics
	{
		CommandList::CommandList()
		{
			Resize(1024);
		}

		CommandList::CommandList(const CommandList& other)
		{
			m_pso = other.m_pso;
			m_commandCount = other.m_commandCount;

			Resize(other.m_commandMaxByteSize);
			m_commandMaxByteSize = other.m_commandMaxByteSize;

			memcpy_s(m_commands, m_commandMaxByteSize, other.m_commands, other.m_commandMaxByteSize);
			m_pos = (Byte*)m_commands + other.GetByteSizeFromStart();
			m_readHead = (Byte*)m_commands + other.GetBytesSizeReadHeadFromStart();

			m_uniformBuffer = other.m_uniformBuffer;
		}

		CommandList::CommandList(CommandList&& other)
		{
			m_pso = other.m_pso;
			m_commandCount = other.m_commandCount;
			m_readHead = other.m_readHead;
			m_pos = other.m_pos;
			m_commandMaxByteSize = other.m_commandMaxByteSize;
			m_commands = other.m_commands;
			m_uniformBuffer = std::move(other.m_uniformBuffer);


			other.Reset();
			other.m_commandMaxByteSize = 0;
			other.m_commands = nullptr;
		}

		CommandList::~CommandList()
		{
			if (m_commands)
			{
				Core::MemoryTracker::Instance().UnTrack(m_commands);
				free(m_commands);
			}
		}

		CommandList& CommandList::operator=(const CommandList& other)
		{
			m_pso = other.m_pso;
			m_commandCount = other.m_commandCount;
			m_commandMaxByteSize = other.m_commandMaxByteSize;

			Resize(m_commandMaxByteSize);
			memcpy_s(m_commands, m_commandMaxByteSize, other.m_commands, other.m_commandMaxByteSize);
			m_pos = (Byte*)m_commands + other.GetByteSizeFromStart();
			m_readHead = (Byte*)m_commands + other.GetBytesSizeReadHeadFromStart();
			return *this;
		}

		CommandList& CommandList::operator=(CommandList&& other)
		{
			m_pso = other.m_pso;
			m_commandCount = other.m_commandCount;
			m_readHead = other.m_readHead;
			m_pos = other.m_pos;
			m_commandMaxByteSize = other.m_commandMaxByteSize;
			m_commands = other.m_commands;

			other.Reset();
			other.m_commandMaxByteSize = 0;
			other.m_commands = nullptr;
			return *this;
		}

		ICommand* CommandList::GetCurrentCommand()
		{
			if (m_readHead == m_pos)
			{
				return nullptr;
			}
			ICommand* currCommand = (ICommand*)m_readHead;
			return currCommand;
		}

		void CommandList::NextCommand()
		{
			ICommand* currCommand = GetCurrentCommand();
			m_readHead += currCommand->GetSize();
		}

		void CommandList::Reset()
		{
			m_pos = (Byte*)m_commands;
			m_commandCount = 0;
			m_readHead = (Byte*)m_commands;
			m_pso = {};
			m_uniformBuffer.Reset();
		}

		void CommandList::ResetReadHead()
		{
			m_readHead = (Byte*)m_commands;
		}

		void CommandList::SetReadHeadToCommandIndex(int index)
		{
			ResetReadHead();
			for (int i = 0; i < index; ++i)
			{
				NextCommand();
			}
		}

		void CommandList::SetPipelineStateObject(PipelineStateObject pso)
		{
			m_pso = pso;
		}

		void CommandList::SetPrimitiveTopologyType(PrimitiveTopologyType type)
		{
			m_pso.PrimitiveTopologyType = type;
		}

		void CommandList::SetPolygonMode(PolygonMode mode)
		{
			m_pso.PolygonMode = mode;
		}

		void CommandList::SetCullMode(CullMode mode)
		{
			m_pso.CullMode = mode;
		}

		void CommandList::SetShader(RHI_Shader* shader)
		{
			m_pso.Shader = shader;
		}

		void CommandList::ClearRenderTargets()
		{
			m_pso.RenderTargets.clear();
		}

		void CommandList::SetViewport(int width, int height)
		{
			AddCommand(CMD_SetViewport(width, height));
		}

		void CommandList::SetScissor(int width, int height)
		{
			AddCommand(CMD_SetScissor(width, height));
		}

		void CommandList::SetUniform(int set, int binding, void* data, int sizeInBytes)
		{
			m_uniformBuffer.SetUniform(set, binding, data, sizeInBytes);
		}

		void CommandList::SetVertexBuffer(RHI_Buffer* buffer)
		{
			AddCommand(CMD_SetVertexBuffer(buffer));
		}

		void CommandList::SetIndexBuffer(RHI_Buffer* buffer)
		{
			AddCommand(CMD_SetIndexBuffer(buffer));
		}

		void CommandList::AddResourceBarrier(ResourceBarrier barrier)
		{
			AddCommand(CMD_AddResourceBarrier(barrier));
		}

		void CommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
		{
			AddCommand(CMD_SetPipelineStateObject(m_pso));
			AddCommand(CMD_Draw(vertexCount, instanceCount, firstVertex, firstInstance));
		}

		void CommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
		{
			AddCommand(CMD_SetPipelineStateObject(m_pso));
			AddCommand(CMD_DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance));
		}

		void CommandList::Resize(u64 newSize)
		{
			if (m_commandMaxByteSize == newSize)
			{
				return;
			}

			u64 byteSizeFromStart = GetByteSizeFromStart();

			if (m_commands != nullptr)
			{
				if (newSize == 0)
				{
					Core::MemoryTracker::Instance().UnTrack(m_commands);
					free(m_commands);
					m_commandMaxByteSize = 0;
					m_commandCount = 0;
					m_pos = 0;
					return;
				}

				Core::MemoryTracker::Instance().UnTrack(m_commands);
				void* newBlock = realloc(m_commands, newSize);
				if (newBlock != nullptr)
				{
					Core::MemoryTracker::Instance().Track(newBlock, Core::MemoryTrackAllocationType::Single);
					m_commands = (ICommand*)newBlock;
					m_pos = (Byte*)m_commands;
					m_pos += byteSizeFromStart;
					m_commandMaxByteSize = newSize;
					// Realloc passed 
					return;
				}
			}

			// Realloc didn't pass, try and alloc with malloc.
			void* newBlock = malloc(newSize);
			assert(newBlock != nullptr && "[CommandList::Resize] Unable to resize command list.");
			Core::MemoryTracker::Instance().Track(newBlock, Core::MemoryTrackAllocationType::Single);

			// Copy all previous commands into new buffer.
			memcpy_s(newBlock, newSize, m_commands, m_commandMaxByteSize);

			// Free old buffer and set command ptr to new buffer.
			free(m_commands);

			m_commands = (ICommand*)newBlock;
			m_pos = (Byte*)m_commands;
			m_pos += byteSizeFromStart;

			m_readHead = (Byte*)m_commands;
			m_commandMaxByteSize = newSize;
		}

		u64 CommandList::GetByteSizeFromStart() const
		{
			if (m_commands != nullptr)
			{
				return (std::uintptr_t)m_pos - (std::uintptr_t)m_commands;
			}
			return 0;
		}

		u64 CommandList::GetBytesSizeReadHeadFromStart() const
		{
			if (m_commands != nullptr)
			{
				return (std::uintptr_t)m_readHead - (std::uintptr_t)m_commands;
			}
			return 0;
		}
	}
}