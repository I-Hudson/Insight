#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_CommandList.h"
#include "Graphics/RHI/DX12/DX12Utils.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

#include "Graphics/RHI/DX12/DescriptorBindingDX12.h"

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

				ID3D12GraphicsCommandList* GetCommandList() const;

				virtual void Create(RenderContext* context) override;
				virtual void PipelineBarrier(Graphics::PipelineBarrier barrier) override;

				void PipelineBarrier(std::vector<D3D12_BUFFER_BARRIER> const& bufferMemoryBarrier, std::vector<D3D12_TEXTURE_BARRIER> const& imageMemoryBarrier);
				void PipelineBarrierBuffer(std::vector<D3D12_BUFFER_BARRIER> const& bufferMemoryBarrier);
				void PipelineBarrierImage(std::vector<D3D12_TEXTURE_BARRIER> const& imageMemoryBarrier);

				void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float* clearColour, int numRects, D3D12_RECT* rects);
				void ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView, D3D12_CLEAR_FLAGS ClearFlags, float Depth, int Stencil, int NumRects, D3D12_RECT* rects);

				void OMSetRenderTargets(int count, D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, bool RTsSingleHandleToDescriptorRange, D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilDescriptor);

				void SetDescriptorHeaps(int count, ID3D12DescriptorHeap** heaps);
				void SetDescriptorHeaps(std::vector<ID3D12DescriptorHeap*> heaps);


				/// RHI_CommandList
				virtual void Reset() override;
				virtual void Close() override;
				virtual void CopyBufferToBuffer(RHI_Buffer* dst, RHI_Buffer* src, u64 offset) override;
				virtual void CopyBufferToImage(RHI_Texture* dst, RHI_Buffer* src, u64 offset) override;

				/// RHI_Resource
				virtual void Release() override;
				virtual bool ValidResource() override;
				virtual void SetName(std::string name) override;

				/// RHI_CommandList
				virtual void BeginRenderpass(RenderpassDescription renderDescription) override;
				virtual void EndRenderpass() override;

				virtual void SetPipeline(PipelineStateObject pso) override;
				virtual void SetPushConstant(u32 offset, u32 size, const void* data) override { ASSERT(false); }

				virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth, bool invert_y = false) override;
				virtual void SetScissor(int x, int y, int width, int height) override;
				virtual void SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor) override;
				virtual void SetLineWidth(float width) override;

				virtual void SetTexture(u32 set, u32 binding, const RHI_Texture* texture) override;
				virtual void SetUniform(u32 set, u32 binding, RHI_BufferView buffer) override;

				virtual void SetVertexBuffer(RHI_Buffer* buffer) override;
				virtual void SetIndexBuffer(RHI_Buffer* buffer, IndexType index_type) override;

				virtual void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) override;
				virtual void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, u32 vertexOffset, u32 firstInstance) override;

				virtual void BindPipeline(PipelineStateObject pso, RHI_DescriptorLayout* layout) override;

				virtual void BeginTimeBlock(const std::string& blockName) override;
				virtual void BeginTimeBlock(const std::string& blockName, glm::vec4 colour) override;
				virtual void EndTimeBlock() override;

			protected:
				virtual bool BindDescriptorSets() override;
				virtual void SetImageLayoutTransition(RHI_Texture* texture, ImageLayout layout) override;


			private:
				RenderContext_DX12* m_contextDX12 = nullptr;

				ID3D12GraphicsCommandList7* m_commandList;
				RHI_CommandListAllocator_DX12* m_allocator{ nullptr };

				DescriptorBindingDX12 m_descriptorBinding;

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
				virtual void SetName(std::string name) override;

			private:
				RenderContext_DX12* m_context{ nullptr };
				ComPtr<ID3D12CommandAllocator> m_allocator{ nullptr };

				std::unordered_map<RHI_CommandList*, std::pair<RHI_CommandList*, ComPtr<ID3D12CommandAllocator>>> m_singleUseCommandLists;
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)