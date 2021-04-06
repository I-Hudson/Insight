#pragma once

#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUCommandBufferVulkan : public GPUResouceVulkan<Graphics::GPUCommandBuffer>
	{
	public:
		GPUCommandBufferVulkan();
		~GPUCommandBufferVulkan();

		virtual void Init(Graphics::GPUCommandBufferDesc const& desc) override;
		virtual void BeginRecord() override;
		virtual void EndRecord() override;
		virtual void Submit() override;
		virtual void Clear() override;

		virtual void BeginRenderpass(Graphics::GPURenderPass* renderpass) override;
		virtual void EndRenderpass(Graphics::GPURenderPass* renderpass) override;
		virtual void SetViewPort(Maths::Rect rect) override;
		virtual void SetScissor(Maths::Rect rect) override;

		virtual void BindDescriptorSets(PipelineBindPoint bindPoint, Graphics::GPUPipelineLayout* pipelineLayout, u32 firstSet, u32 descriptorSetCount, Graphics::GPUDescriptorSet* descriptorSets, u32 dynamicOffsetCount, u32 const* dynamicOffsets) override;
		virtual void BindVertexBuffers(u32 firstBinding, u32 bindingCount, GPUBuffer* buffers, u32* offsets) override;
		virtual void BindIndexBuffers(GPUBuffer* buffer, u32 offset, Graphics::GPUCommandBufferIndexType indexType) override;
		virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

		// [GPUResouce]
		virtual void OnReleaseGPU() override;

	private:
		VkCommandBuffer m_cmdBuffer;
	};
}