#pragma once
#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Core/Maths/Rect.h"

namespace Insight::Graphics
{
	class GPUBuffer;
	class GPURenderPass;
	class GPUPipelineLayout;
	class GPUDescriptorSet;

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

	enum class GPUCommandBufferIndexType
	{
		UINT16,
		UINT32
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
		GPUCommandBuffer();
		GPUCommandBuffer(GPUCommandBuffer const& other) = delete;
		~GPUCommandBuffer();

		static GPUCommandBuffer* New();
		
		GPUCommandBufferDesc const& GetDesc() const { return m_desc; }
		bool const& IsSubmitted() const { return m_state == GPUCommandBufferState::SUBMITTED; }
		u32 const& GetRecordCommandsCount() const { return m_recordCommandCount; }
		GPUCommandBufferState const& GetState() const { return m_state; }

		virtual void Init(GPUCommandBufferDesc const& desc) = 0;
		virtual void BeginRecord() = 0;
		virtual void EndRecord() = 0;
		virtual void Submit() = 0;
		virtual void Clear() = 0;

		virtual void BeginRenderpass(GPURenderPass* renderpass) = 0;
		virtual void EndRenderpass(GPURenderPass* renderpass) = 0;
		virtual void SetViewPort(Maths::Rect rect) = 0;
		virtual void SetScissor(Maths::Rect rect) = 0;

		virtual void BindDescriptorSets(PipelineBindPoint bindPoint, GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets) = 0;
		virtual void BindVertexBuffers(u32 firstBinding, u32 bindingCount, GPUBuffer* buffers, u32* offsets) = 0;
		virtual void BindIndexBuffer(GPUBuffer* buffer, u32 offset, GPUCommandBufferIndexType indexType) = 0;
		virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) = 0;

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::CommandBuffer; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUCommandBufferDesc m_desc;
		u32 m_recordCommandCount;
		GPUCommandBufferState m_state;
	};

}
