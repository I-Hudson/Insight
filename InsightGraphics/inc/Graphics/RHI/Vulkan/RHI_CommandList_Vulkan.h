#pragma once

#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

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

				// RHI_CommandList
				virtual void Reset() override;
				virtual void Close() override;

				// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			protected:
				// RHI_CommandList
				virtual void SetPipeline(PipelineStateObject pso) override;
				virtual void SetUniform(int set, int binding, DescriptorBufferView view) override;
				virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) override;
				virtual void SetScissor(int x, int y, int width, int height) override;

				virtual void SetVertexBuffer() override;
				virtual void SetIndexBuffer() override;

				virtual void Draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance) override;
				virtual void DrawIndexed(int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance) override;

				virtual void BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout) override;

				virtual bool BindDescriptorSets() override;

			private:
				RenderContext_Vulkan* RenderContextVulkan();
				FrameResource_Vulkan* FrameResourceVulkan();

			private:
				vk::CommandBuffer m_commandList{ nullptr };
				RHI_CommandListAllocator_Vulkan* m_allocator{ nullptr };
				vk::Framebuffer m_framebuffer{ nullptr };

				friend class RHI_CommandListAllocator_Vulkan;
			};

			class RHI_CommandListAllocator_Vulkan : public RHI_CommandListAllocator
			{
			public:

				vk::CommandPool GetAllocator() const { return m_allocator; }

				// RHI_CommandListAllocator
				virtual void Create(RenderContext* context) override;

				virtual RHI_CommandList* GetCommandList() override;
				virtual void Reset() override;

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