#pragma once

#include "Graphics/CommandList.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;
			class CommandAllocator_DX12;

			class CommandList_DX12
			{
			public:
				CommandList_DX12() { }
				CommandList_DX12(RenderContext_DX12* context, ComPtr<ID3D12GraphicsCommandList> cmdList);

				void Record(CommandList& cmdList);
				void Reset();
				ID3D12GraphicsCommandList* GetCommandBuffer() const { return m_commandList.Get(); }

				bool operator==(const CommandList_DX12& other) const { return m_commandList == other.m_commandList; }

			private:
				bool CanDraw();

			private:
				ComPtr<ID3D12GraphicsCommandList> m_commandList;
				RenderContext_DX12* m_context;

				bool m_activeRenderpass = false;
				PipelineStateObject m_pso;
				PipelineStateObject m_activePSO;

				friend class CommandAllocator_DX12;
			};

			class CommandAllocator_DX12
			{
			public:

				void Init(RenderContext_DX12* context);
				void Update();
				void Destroy();

				CommandList_DX12& GetCommandList();
				void ReturnCommandList(CommandList_DX12& cmdList);

			private:
				RenderContext_DX12* m_context{ nullptr };

				std::vector<CommandList_DX12> m_allocLists;
				std::vector<CommandList_DX12> m_freeLists;

				ComPtr<ID3D12CommandAllocator> m_allocator{ nullptr };
			};
		}
	}
}