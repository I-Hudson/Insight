#pragma once
#include "Engine/Graphics/GPUResource.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Core/Maths/Rect.h"


namespace Insight::Graphics
{
	class GPUBuffer;
	class GPURenderPass;
	class GPUCommandPool;
	class GPUPipelineLayout;
	class GPUDescriptorSet;

	enum class GPUCommandBufferState : u8
	{
		IDLE,
		RECORDING,
		SUBMITTED,

		COUNT
	};

	enum class GPUCommandBufferUsageFlags
	{
		INVALID,
		ONE_TIME_SUBMIT, 
		RENDER_PASS_CONTINUE,
		SIMULATANEOUS_USE
	};

	enum class GPUCommandBufferLevel
	{
		PRIMARY,
		SECONDARY
	};

	enum class GPUCommandBufferIndexType
	{
		UINT16,
		UINT32
	};

	struct GPUCommandBufferDesc
	{
		GPUCommandBufferDesc(GPUCommandBufferUsageFlags const& flags, GPUCommandPool* commandPool = nullptr, 
							 GPUCommandBufferLevel const& level = GPUCommandBufferLevel::PRIMARY, bool const& releaseResources = false)
			: Usage(flags)
			, CommandPool(commandPool)
			, Level(level)
			, ReleaseResources(releaseResources)
		{ }

		GPUCommandBufferUsageFlags Usage = GPUCommandBufferUsageFlags::INVALID;
		GPUCommandBufferLevel Level = GPUCommandBufferLevel::PRIMARY;
		GPUCommandPool* CommandPool = nullptr;
		bool ReleaseResources = false;

		static GPUCommandBufferDesc CreateOneTimeCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::ONE_TIME_SUBMIT, nullptr, GPUCommandBufferLevel::PRIMARY, false);
		}
		static GPUCommandBufferDesc CreateRenderPassContinueCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::RENDER_PASS_CONTINUE, nullptr, GPUCommandBufferLevel::PRIMARY, false);
		}
		static GPUCommandBufferDesc CreateSimulataneousUseCmdBuffer()
		{
			return GPUCommandBufferDesc(GPUCommandBufferUsageFlags::SIMULATANEOUS_USE, nullptr, GPUCommandBufferLevel::PRIMARY, false);
		}
	};

	enum class GPUCommandPoolFlags
	{
		INVALID, 
		TRANSIENT,
		RESET_COMMAND_BUFFER,
		PROTECTED
	};

	struct GPUCommandPoolDesc
	{
		GPUCommandPoolDesc(GPUCommandPoolFlags const& flags, GPUQueue queue)
			: Flags(flags), Queue(queue)
		{ }
		GPUCommandPoolFlags Flags;
		GPUQueue Queue;
	};

	/// <summary>
	/// Interface for all command buffers.
	/// </summary>
	class GPUCommandBuffer : public GPUResource
	{
	public:
		GPUCommandBuffer();
		GPUCommandBuffer(GPUCommandBuffer const& other) = delete;
		virtual ~GPUCommandBuffer();

		static GPUCommandBuffer* New();
		
		GPUCommandBufferDesc const& GetDesc() const { return m_desc; }
		bool const& IsSubmitted() const { return m_state == GPUCommandBufferState::SUBMITTED; }
		u32 const& GetRecordCommandsCount() const { return m_recordCommandCount; }
		GPUCommandBufferState const& GetState() const { return m_state; }
		bool const IsEmpty() const { return m_recordCommandCount == 0; }

		virtual void Init(GPUCommandBufferDesc const& desc) = 0;
		virtual void BeginRecord() = 0;
		virtual void EndRecord() = 0;
		virtual void Reset() = 0;
		virtual void Submit(GPUQueue queue) = 0;
		virtual void SubmitAndWait(GPUQueue queue) = 0;

		virtual void BeginRenderpass(GPURenderPass* renderpass) = 0;
		virtual void EndRenderpass(GPURenderPass* renderpass) = 0;
		virtual void SetViewPort(Maths::Rect rect) = 0;
		virtual void SetScissor(Maths::Rect rect) = 0;

		virtual void CopyBuffer(GPUBuffer* srcBuffer, GPUBuffer* dstBuffer, u32 regionCount, u64 srcOffset, u64 dstOffset, u64 size) = 0;

		virtual void BindDescriptorSets(PipelineBindPoint bindPoint, GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets) = 0;
		virtual void BindVertexBuffers(u32 firstBinding, u32 bindingCount, GPUBuffer** buffers, u32* offsets) = 0;
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

	class GPUCommandPool : public GPUResource
	{
	public:
		GPUCommandPool();
		virtual ~GPUCommandPool();

		static GPUCommandPool* New();

		virtual void Init(GPUCommandPoolDesc const& desc) = 0;
		virtual GPUCommandBuffer* AllocateCommandBuffer(GPUCommandBufferDesc& desc) = 0;
		virtual std::vector<GPUCommandBuffer*> AllocateCommandBuffers(GPUCommandBufferDesc& desc, u32 count) = 0;
		virtual void FreeCommandBuffer(std::vector<GPUCommandBuffer*> buffers) = 0;

		// [GPUResource]
		virtual ResourceType GetResourceType() const override { return ResourceType::CommandPool; }
		virtual ObjectType GetObjectType() const override { return ObjectType::Other; }

	protected:
		GPUCommandPoolDesc m_desc;
		std::vector<GPUCommandBuffer*> m_buffers;
	};
}
