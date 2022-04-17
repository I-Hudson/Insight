#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/PipelineStateObject.h"

namespace Insight
{
	namespace Graphics
	{
		class GPUBuffer;
		struct ResourceBarrier
		{
			ResourceType Type;
			void* Resource;
			ResourceState InialState;
			ResourceState FinalState;
		};

		enum class CommandType
		{
			None,
			SetPipelineStateObject,
			SetViewport,
			SetScissor,
			SetVertexBuffer,
			SetIndexBuffer,
			AddResourceBarrier,
			Draw, 
			DrawIndexed,
		};

		struct ICommand
		{
			virtual u64 GetSize() = 0;
			CommandType CommandType;
		};

		struct CMD_SetPipelineStateObject : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetPipelineStateObject); }
			CMD_SetPipelineStateObject(PipelineStateObject pso) 
			{
				CommandType = CommandType::SetPipelineStateObject;
				Pso = pso;
			}
			PipelineStateObject Pso;
		};

		struct CMD_SetViewport : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetViewport); }
			CMD_SetViewport(int width, int height)
			{ 
				CommandType = CommandType::SetViewport; 
				Width = width;
				Height = height;
			}

			int Width;
			int Height;
		};

		struct CMD_SetScissor : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetScissor); }
			CMD_SetScissor(int width, int height)
			{
				CommandType = CommandType::SetScissor;
				Width = width;
				Height = height;
			}

			int Width;
			int Height;
		};


		struct CMD_SetVertexBuffer : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetVertexBuffer); }
			CMD_SetVertexBuffer(GPUBuffer* buffer)
			{ 
				CommandType = CommandType::SetVertexBuffer;
				Buffer = buffer;
			}
			GPUBuffer* Buffer;
		};

		struct CMD_SetIndexBuffer : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetIndexBuffer); }
			CMD_SetIndexBuffer(GPUBuffer* buffer)
			{
				CommandType = CommandType::SetIndexBuffer;
				Buffer = buffer;
			}
			GPUBuffer* Buffer;
		};

		struct CMD_Draw : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_Draw); }
			CMD_Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
			{
				CommandType = CommandType::Draw;
				VertexCount = vertexCount;
				InstanceCount = instanceCount;
				FirstVertex = firstVertex;
				FirstInstance = firstInstance;
			}
			u32 VertexCount;
			u32 InstanceCount;
			u32 FirstVertex;
			u32 FirstInstance;
		};

		struct CMD_DrawIndexed : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_DrawIndexed); }
			CMD_DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance)
			{
				CommandType = CommandType::DrawIndexed;
				IndexCount = indexCount;
				InstanceCount = instanceCount;
				FirstIndex = firstIndex;
				VertexOffset = vertexOffset;
				FirstInstance = firstInstance;
			}
			u32 IndexCount;
			u32 InstanceCount;
			u32 FirstIndex;
			u32 VertexOffset;
			u32 FirstInstance;
		};

		struct CMD_AddResourceBarrier : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_AddResourceBarrier); }
			CMD_AddResourceBarrier(ResourceBarrier barrier)
			{
				CommandType = CommandType::AddResourceBarrier;
				Barrier = barrier;
			}
			ResourceBarrier Barrier;
		};
	}
}