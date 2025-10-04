#pragma once
#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_PipelineManager.h"

#include <d3d12.h>

namespace Insight
{
	namespace Graphics
	{
		class RenderContext;

		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class RHI_PipelineCache_DX12 : public RHI_PipelineCahce
			{
			public:
				RHI_PipelineCache_DX12(RenderContext* context);
				virtual ~RHI_PipelineCache_DX12() override;

				virtual RHI_Pipeline* GetGraphicsPSO(PipelineStateObject& pso) const override;
				virtual void AddPSO(const PipelineStateObject& pso, const RHI_Pipeline* pipeline) override;

			private:
				ID3D12PipelineLibrary1* m_library = nullptr;
				RenderContext_DX12* m_context = nullptr;
			};
		}
	}
}
#endif