#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Core/Slice.h"

#include <mutex>

#include <vulkan/vulkan.h>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;
			struct FrameResource_Vulkan;
			class RHI_CommandListAllocator_Vulkan;

			class RHI_CommandList_Vulkan : public RHI_CommandList
			{
			public:

				VkCommandBuffer GetCommandList() const { return m_commandList; };

				virtual void Create(RenderContext* context) override;
				virtual void PipelineBarrier(Graphics::PipelineBarrier barrier) override;

				void PipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage
					, std::vector<VkBufferMemoryBarrier> const& bufferMemoryBarrier, std::vector<VkImageMemoryBarrier> const& imageMemoryBarrier);
				void PipelineBarrierBuffer(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<VkBufferMemoryBarrier> const& bufferMemoryBarrier);
				void PipelineBarrierImage(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<VkImageMemoryBarrier> const& imageMemoryBarrier);

				/// RHI_CommandList
				virtual void Reset() override;
				virtual void Close() override;
				virtual void CopyBufferToBuffer(RHI_Buffer* dst, u64 dstOffset, RHI_Buffer* src, u64 srcOffset, u64 sizeInBytes) override;
				virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;

				/// RHI_CommandList
				virtual void BeginRenderpass(RenderpassDescription renderDescription) override;
				virtual void EndRenderpass() override;

				virtual void SetPipeline(PipelineStateObject pso) override;
				virtual void SetPushConstant(u32 offset, u32 size, const void* data) override;

				virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y = false) override;
				virtual void SetScissor(int x, int y, int width, int height) override;
				virtual void SetLineWidth(float width) override;

				virtual void SetVertexBuffer(const RHI_BufferView& bufferView) override;
				virtual void SetIndexBuffer(const RHI_BufferView& bufferView, const IndexType index_type) override;

				virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
				virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

				virtual void Dispatch(const u32 threadGroupX, const u32 threadGroupY) override;

				virtual void BindPipeline(const PipelineStateObject& pso, RHI_DescriptorLayout* layout) override;
				virtual void BindPipeline(const PipelineStateObject& pso, bool clearDescriptors) override;
				virtual void BindPipeline(const ComputePipelineStateObject& pso) override;

				virtual void BeginTimeBlock(const std::string& blockName) override;
				virtual void BeginTimeBlock(const std::string& blockName, Maths::Vector4 colour) override;
				virtual void EndTimeBlock() override;

			protected:
				virtual bool BindDescriptorSets(const GPUQueue gpuQueue) override;
				virtual void SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout) override;

			private:
				void CreateFramebuffer(VkRenderPass renderpass, VkRect2D rect, std::vector<VkClearValue>& clearColours);

			private:
				RenderContext_Vulkan* m_context_vulkan = nullptr;

				VkCommandBuffer m_commandList{ nullptr };
				RHI_CommandListAllocator_Vulkan* m_allocator{ nullptr };
				std::unordered_map<u64, VkFramebuffer> m_framebuffers;

				bool m_dynamicRendering;
				VkPipelineLayout m_bound_pipeline_layout;
				u64 m_boundDescriptors;

				PFN_vkCmdBeginDebugUtilsLabelEXT m_cmdBeginDebugUtilsLabelEXT;
				PFN_vkCmdEndDebugUtilsLabelEXT m_cmdEndDebugUtilsLabelEXT;
				VkDebugUtilsLabelEXT m_activeDebugUtilsLabel = VkDebugUtilsLabelEXT();

				friend class RenderContext_Vulkan;
				friend class RHI_CommandListAllocator_Vulkan;
			};

			class RHI_CommandListAllocator_Vulkan : public RHI_CommandListAllocator
			{
				THREAD_SAFE
			public:
				THREAD_SAFE;
				VkCommandPool GetAllocator() const { return m_allocator; }

				/// RHI_CommandListAllocator
				virtual void Create(RenderContext* context, const RHI_CommandListAllocatorDesc desc) override;
				
				virtual void Reset() override;

				virtual RHI_CommandList* GetCommandList() override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				VkCommandPool m_allocator{ nullptr };
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)