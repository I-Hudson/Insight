#pragma once

#include "Graphics/PipelineStateObject.h"
#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "Graphics/RHI/DX12/RHI_Descriptor_DX12.h"
#include <map>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class PipelineStateObjectManager_DX12
			{
			public:
				PipelineStateObjectManager_DX12();
				~PipelineStateObjectManager_DX12();

				void SetRenderContext(RenderContext_DX12* context) { m_context = context; }
				ID3D12PipelineState* GetOrCreatePSO(PipelineStateObject pso);
				void Destroy();

			private:
				std::map<u64, ComPtr<ID3D12PipelineState>> m_pipelines;
				RenderContext_DX12* m_context{ nullptr };
			};
		}
	}
}