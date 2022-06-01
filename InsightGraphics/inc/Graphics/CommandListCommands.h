#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/PipelineStateObject.h"
#include "Graphics/RHI/RHI_Descriptor.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;
		//struct DescriptorBufferView;
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
			SetUniformBuffer,
			SetUniform,
			SetTexture,
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
				Pso = std::move(pso);
			}
			PipelineStateObject Pso;
		};

		struct CMD_SetDescriptorBuffer : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetDescriptorBuffer); }
			CMD_SetDescriptorBuffer(DescriptorBuffer buffer)
			{
				CommandType = CommandType::SetUniformBuffer;
				Buffer = std::move(buffer);
			}
			DescriptorBuffer Buffer;
		};

		struct CMD_SetUniform : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetUniform); }
			CMD_SetUniform(int set, int binding, DescriptorBufferView view)
			{
				CommandType = CommandType::SetUniform;
				Set = set;
				Binding = binding;
				View = view;
			}
			int Set;
			int Binding;
			DescriptorBufferView View;
		};

		struct CMD_SetTexture : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetTexture); }
			CMD_SetTexture(int set, int binding, RHI_Texture* texture)
			{
				CommandType = CommandType::SetTexture;
				Set = set;
				Binding = binding;
				Texture = texture;
			}
			int Set;
			int Binding;
			RHI_Texture* Texture;
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
			CMD_SetVertexBuffer(RHI_Buffer* buffer)
			{ 
				CommandType = CommandType::SetVertexBuffer;
				Buffer = buffer;
			}
			RHI_Buffer* Buffer;
		};

		struct CMD_SetIndexBuffer : public ICommand
		{
			virtual u64 GetSize() override { return sizeof(CMD_SetIndexBuffer); }
			CMD_SetIndexBuffer(RHI_Buffer* buffer)
			{
				CommandType = CommandType::SetIndexBuffer;
				Buffer = buffer;
			}
			RHI_Buffer* Buffer;
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