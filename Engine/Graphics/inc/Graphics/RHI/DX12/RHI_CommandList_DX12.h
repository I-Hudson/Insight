#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;
			class RHI_CommandListAllocator_DX12;

			class RHI_CommandList_DX12 : public RHI_CommandList
			{
			public:

				virtual void Create(RenderContext* context) override { ASSERT(false); }
				virtual void PipelineBarrier(Graphics::PipelineBarrier barrier) override { ASSERT(false); }

				void ResourceBarrier(int count, D3D12_RESOURCE_BARRIER* barriers);
				void ResourceBarrierImage(ID3D12Resource* resource, D3D12_RESOURCE_STATES srcState, D3D12_RESOURCE_STATES dstState);

				void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float* clearColour, int numRects, D3D12_RECT* rects);
				void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, int Stencil, int NumRects, D3D12_RECT* rects);

				void OMSetRenderTargets(int count, D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, bool RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);

				void SetDescriptorHeaps(int count, ID3D12DescriptorHeap** heaps);
				void SetDescriptorHeaps(std::vector<ID3D12DescriptorHeap*> heaps);

				ID3D12GraphicsCommandList* GetCommandList() const;

				/// RHI_CommandList
				virtual void Reset() override;
				virtual void Close() override;
				virtual void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset) override;
				virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::wstring name) override;

				/// RHI_CommandList
				virtual void BeginRenderpass(RenderpassDescription renderDescription) override;
				virtual void EndRenderpass() override;

				virtual void SetPipeline(PipelineStateObject pso) override;
				virtual void SetPushConstant(u32 offset, u32 size, const void* data) override { ASSERT(false); }

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
				RenderContext_DX12* RenderContextDX12();

			private:
				ComPtr<ID3D12GraphicsCommandList> m_commandList;
				RHI_CommandListAllocator_DX12* m_allocator{ nullptr };

				std::unordered_map<u64, D3D12_RESOURCE_BARRIER> m_ResourceBarriers;

				friend class RHI_CommandListAllocator_DX12;
			};

			class RHI_CommandListAllocator_DX12 : public RHI_CommandListAllocator
			{
			public:

				ID3D12CommandAllocator* GetAllocator() const { return m_allocator.Get(); }

				/// RHI_CommandListAllocator
				virtual void Create(RenderContext* context) override;
				
				virtual void Reset() override;

				virtual RHI_CommandList* GetCommandList() override;
				virtual RHI_CommandList* GetSingleSubmitCommandList() override;

				virtual void ReturnSingleSubmitCommandList(RHI_CommandList* cmdList) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::wstring name) override;

			private:
				RenderContext_DX12* m_context{ nullptr };
				ComPtr<ID3D12CommandAllocator> m_allocator{ nullptr };

				std::unordered_map<RHI_CommandList*, std::pair<RHI_CommandList*, ComPtr<ID3D12CommandAllocator>>> m_singleUseCommandLists;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)