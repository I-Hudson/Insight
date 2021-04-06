#pragma once
#include "Engine/Graphics/GPUResource.h"
#include "Engine/Core/Maths/Rect.h"

namespace Insight::Graphics
{
	class GPURenderPass;

	enum class GPUCommandBufferState : u8
	{
		IDLE,
		RECORDING,
		SUBMITTED,

		COUNT
	};

	enum GPUCommandBufferUsageFlags
	{
		INVALID,

		ONE_TIME_SUBMIT, 
		RENDER_PASS_CONTINUE,
		SIMULATANEOUS_USE
	};

	struct GPUCommandBufferDesc
	{
		GPUCommandBufferDesc(GPUCommandBufferUsageFlags const& flags)
			: Usage(flags)
		{ }
		GPUCommandBufferUsageFlags Usage = GPUCommandBufferUsageFlags::INVALID;

		static GPUCommandBufferDesc CreateOneTimeCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::ONE_TIME_SUBMIT);
		}
		static GPUCommandBufferDesc CreateRenderPassContinueCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::RENDER_PASS_CONTINUE);
		}
		static GPUCommandBufferDesc CreateSimulataneousUseCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::SIMULATANEOUS_USE);
		}
	};

	/// <summary>
	/// Interface for all command buffers.
	/// </summary>
	class GPUCommandBuffer : public GPUResource
	{
	public:
		GPUCommandBuffer()
		: m_isUsed(false)
		, m_recordCommandCount(0)
		, m_state(GPUCommandBufferState::IDLE)
		{ }
		~GPUCommandBuffer() { }

		static GPUCommandBuffer* New();

		virtual void Init(GPUCommandBufferDesc const& desc) = 0;
		virtual void BeginRecord() = 0;
		virtual void EndRecord() = 0;
		virtual void Submit() = 0;
		virtual void Clear() = 0;

		virtual ResourceType GetResourceType() const override { return ResourceType::CommandBuffer; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Buffer; }

		bool const& IsUsed() const { return m_isUsed; }
		u32 const& GetRecordCommandsCount() const { return m_recordCommandCount; }
		GPUCommandBufferState const& GetState() const { return m_state; }

		virtual void BeginRenderpass(GPURenderPass* renderpass) = 0;
		virtual void EndRenderpass(GPURenderPass* renderpass) = 0;
		virtual void SetViewPort(Maths::Rect rect) = 0;
		virtual void SetScissor(Maths::Rect rect) = 0;

		virtual void BindDescriptorSets(bindPoint, pipelineLayout, u32 firstSet, u32 descriptorSetCount, descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets) = 0;
		virtual void BindVertexBuffer() = 0;
		virtual void BindVertexBuffers() = 0;
		virtual void BindIndexBuffer() = 0;
		virtual void BindIndexBuffers() = 0;
		virtual void DrawIndex(u32 indicesCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) = 0;

	protected:
		bool m_isUsed;
		u32 m_recordCommandCount;
		GPUCommandBufferState m_state;
	};

}
