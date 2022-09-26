#pragma once

#include "Graphics/GPU/GPUCommandList.h"
#include "Graphics/GPU/RHI/DX12/GPUDevice_DX12.h"

#include <map>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class GPUComamndListAllocator_DX12;

			class GPUCommandList_DX12 : public GPUCommandList, public GPUResource_DX12
			{
			public:
				GPUCommandList_DX12();
				virtual ~GPUCommandList_DX12() override;

				virtual void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, u64 srcOffset, u64 dstOffset, u64 size) override;

				virtual void SetViewport(int width, int height) override;
				virtual void SetScissor(int width, int height) override;

				virtual void SetVertexBuffer(GPUBuffer* buffer) override;
				virtual void SetIndexBuffer(GPUBuffer* buffer) override;

				virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
				virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

				virtual void Submit(GPUQueue queue, std::vector<GPUSemaphore*> waitSemaphores, std::vector<GPUSemaphore*> signalSemaphores) override;

				virtual void BeginRecord() override;
				virtual void EndRecord() override;

				virtual void BeginRenderpass() override;
				virtual void EndRenderpass() override;

				virtual void BindPipeline(GPUPipelineStateObject* pipeline) override;

				ID3D12GraphicsCommandList* GetCommandListDX12() { return m_commandList.Get(); }

			private:
				ComPtr<ID3D12GraphicsCommandList> m_commandList{ nullptr };
				GPUComamndListAllocator_DX12* m_allocator{ nullptr };

				friend class GPUComamndListAllocator_DX12;
			};

			class GPUComamndListAllocator_DX12 : public GPUComamndListAllocator, public GPUResource_DX12
			{
			public:
				GPUComamndListAllocator_DX12();
				virtual ~GPUComamndListAllocator_DX12() override;

				virtual GPUCommandList* AllocateCommandList(GPUCommandListType type) override;

				virtual void ResetCommandLists(std::list<GPUCommandList*> cmdLists) override;
				virtual void ResetCommandPool(GPUCommandListType type) override;

				virtual void FreeCommandList(GPUCommandList* cmdList) override;
				virtual void FreeCommandLists(const std::list<GPUCommandList*>& cmdLists) override;
				virtual void FreeAllCommandLists() override;

				ID3D12CommandAllocator* GetCommandPool(GPUCommandListType type) const;

			protected:
				virtual void Destroy() override;

			private:
				std::map<GPUCommandListType, ComPtr<ID3D12CommandAllocator>> m_commandPools;
			};
		}
	}
}