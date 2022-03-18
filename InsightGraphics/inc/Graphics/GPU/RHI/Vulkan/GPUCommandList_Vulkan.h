#pragma once

#include "Graphics/GPU/GPUCommandList.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

#include <map>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUComamndListAllocator_Vulkan;

			class GPUCommandList_Vulkan : public GPUCommandList, public GPUResource_Vulkan
			{
			public:
				GPUCommandList_Vulkan();
				virtual ~GPUCommandList_Vulkan() override;

				virtual void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, u64 srcOffset, u64 dstOffset, u64 size) override;

				virtual void SetViewport(int width, int height) override;
				virtual void SetScissor(int width, int height) override;

				virtual void SetVertexBuffer(GPUBuffer* buffer) override;
				virtual void SetIndexBuffer(GPUBuffer* buffer) override;

				virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
				virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

				virtual void Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores, GPUFence* fence) override;

				virtual void BeginRecord() override;
				virtual void EndRecord() override;

				virtual void BeginRenderpass() override;
				virtual void EndRenderpass() override;

				virtual void BindPipeline(GPUPipelineStateObject* pipeline) override;

				vk::CommandBuffer GetCommandBufferVulkan() { return m_commandList; }

			private:
				vk::CommandBuffer m_commandList{ nullptr };
				vk::Framebuffer m_framebuffer;

				friend class GPUComamndListAllocator_Vulkan;
			};

			class GPUComamndListAllocator_Vulkan : public GPUComamndListAllocator, public GPUResource_Vulkan
			{
			public:
				GPUComamndListAllocator_Vulkan();
				virtual ~GPUComamndListAllocator_Vulkan() override;

				virtual GPUCommandList* AllocateCommandList(GPUCommandListType type) override;

				virtual void ResetCommandLists(std::list<GPUCommandList*> cmdLists) override;
				virtual void ResetCommandPool(GPUCommandListType type) override;

				virtual void FreeCommandList(GPUCommandList* cmdList) override;
				virtual void FreeCommandLists(const std::list<GPUCommandList*>& cmdLists) override;
				virtual void FreeAllCommandLists() override;

				virtual void Destroy() override;

			private:
				std::map<GPUCommandListType, vk::CommandPool> m_commandPools;
			};
		}
	}
}