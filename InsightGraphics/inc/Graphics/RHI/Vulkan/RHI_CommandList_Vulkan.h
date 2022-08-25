#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Core/Slice.h"

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

				vk::CommandBuffer GetCommandList() const { return m_commandList; };

				virtual void Create(RenderContext* context) override;
				virtual void PipelineBarrier(Graphics::PipelineBarrier barrier) override;

				void PipelineBarrier(PipelineStageFlags srcStage, PipelineStageFlags dstStage
					, std::vector<vk::BufferMemoryBarrier> const& bufferMemoryBarrier, std::vector<vk::ImageMemoryBarrier> const& imageMemoryBarrier);
				void PipelineBarrierBuffer(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<vk::BufferMemoryBarrier> const& bufferMemoryBarrier);
				void PipelineBarrierImage(PipelineStageFlags srcStage, PipelineStageFlags dstStage, std::vector<vk::ImageMemoryBarrier> const& imageMemoryBarrier);

				// RHI_CommandList
				virtual void Reset() override;
				virtual void Close() override;
				virtual void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset) override;
				virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src) override;

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

				// RHI_CommandList
				virtual void BeginRenderpass(RenderpassDescription renderDescription) override;
				virtual void EndRenderpass() override;

				virtual void SetPipeline(PipelineStateObject pso) override;
				virtual void SetPushConstant(u32 offset, u32 size, void* data) override;

				virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y = false) override;
				virtual void SetScissor(int x, int y, int width, int height) override;

				virtual void SetVertexBuffer(RHI_Buffer* buffer) override;
				virtual void SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type) override;

				virtual void Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance) override;
				virtual void DrawIndexed(int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance) override;

				virtual void BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout) override;

			protected:
				virtual bool BindDescriptorSets() override;

			private:
				void CreateFramebuffer(vk::RenderPass renderpass, vk::Rect2D rect, std::vector<vk::ClearValue>& clearColours);

			private:
				RenderContext_Vulkan* m_context_vulkan = nullptr;

				vk::CommandBuffer m_commandList{ nullptr };
				RHI_CommandListAllocator_Vulkan* m_allocator{ nullptr };
				std::unordered_map<u64, vk::Framebuffer> m_framebuffers;

				u64 m_boundDescriptors;

				friend class RHI_CommandListAllocator_Vulkan;
			};

			class RHI_CommandListAllocator_Vulkan : public RHI_CommandListAllocator
			{
			public:

				vk::CommandPool GetAllocator() const { return m_allocator; }

				// RHI_CommandListAllocator
				virtual void Create(RenderContext* context) override;
				
				virtual void Reset() override;

				virtual RHI_CommandList* GetCommandList() override;
				virtual RHI_CommandList* GetSingleSubmitCommandList() override;

				virtual void ReturnSingleSubmitCommandList(RHI_CommandList* cmdList) override;

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				vk::CommandPool m_allocator{ nullptr };
			};
		}
	}
}

#endif //#if defined(IS_VULKAN_ENABLED)