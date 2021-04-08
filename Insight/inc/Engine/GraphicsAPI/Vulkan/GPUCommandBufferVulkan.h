#pragma once

#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandPoolVulkan;

	class GPUCommandBufferVulkan : public GPUResouceVulkan<Graphics::GPUCommandBuffer>
	{
	public:
		GPUCommandBufferVulkan();
		virtual ~GPUCommandBufferVulkan() override;

		virtual void Init(Graphics::GPUCommandBufferDesc const& desc) override;
		virtual void BeginRecord() override;
		virtual void EndRecord() override;
		virtual void Reset() override;
		virtual void Submit(GPUQueue queue) override;
		virtual void SubmitAndWait(GPUQueue queue) override;

		virtual void BeginRenderpass(Graphics::GPURenderPass* renderpass) override;
		virtual void EndRenderpass(Graphics::GPURenderPass* renderpass) override;
		virtual void SetViewPort(Maths::Rect rect) override;
		virtual void SetScissor(Maths::Rect rect) override;

		virtual void CopyBuffer(Graphics::GPUBuffer* srcBuffer, Graphics::GPUBuffer* dstBuffer, u32 regionCount, u64 srcOffset, u64 dstOffset, u64 size) override;

		virtual void BindDescriptorSets(PipelineBindPoint bindPoint, Graphics::GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, Graphics::GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets) override;
		virtual void BindVertexBuffers(u32 firstBinding, u32 bindingCount, Graphics::GPUBuffer** buffers, u32* offsets) override;
		virtual void BindIndexBuffer(Graphics::GPUBuffer* buffer, u32 offset, Graphics::GPUCommandBufferIndexType indexType) override;
		virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

		// [GPUResouce]
		virtual void OnReleaseGPU() override;

	private:
		VkCommandBuffer m_cmdBuffer;

		friend class GPUCommandPoolVulkan;
	};

	class GPUCommandPoolVulkan : public GPUResouceVulkan<Graphics::GPUCommandPool>
	{
	public:
		GPUCommandPoolVulkan();
		virtual ~GPUCommandPoolVulkan() override;

		virtual void Init(Graphics::GPUCommandPoolDesc const& desc) override;
		virtual Graphics::GPUCommandBuffer* AllocateCommandBuffer(Graphics::GPUCommandBufferDesc& desc) override;
		virtual std::vector<Graphics::GPUCommandBuffer*> AllocateCommandBuffers(Graphics::GPUCommandBufferDesc& desc, u32 count) override;
		virtual void FreeCommandBuffer(std::vector<Graphics::GPUCommandBuffer*> buffers) override;

		// [GPUResource]
		virtual void OnReleaseGPU() override;

	private:
		void AllocateCommandBuffer(GPUCommandBufferVulkan* buffer);
		void FreeCommandBuffer(GPUCommandBufferVulkan* buffer);

	private:
		VkCommandPool m_cmdPool;

		friend class GPUCommandBufferVulkan;
	};
}