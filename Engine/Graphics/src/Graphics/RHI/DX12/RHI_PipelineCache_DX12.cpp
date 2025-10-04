#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_PipelineCache_DX12.h"
#include "Graphics/RHI/DX12/RHI_Pipeline_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"

#include "Platforms/Platform.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_PipelineCache_DX12::RHI_PipelineCache_DX12(RenderContext* context)
				: m_context(static_cast<RenderContext_DX12*>(context))
			{
				const HRESULT hr = m_context->GetDevice()->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(&m_library));
				ASSERT(SUCCEEDED(hr));
			}

			RHI_PipelineCache_DX12::~RHI_PipelineCache_DX12()
			{
			}

			RHI_Pipeline* RHI_PipelineCache_DX12::GetGraphicsPSO(PipelineStateObject& pso) const
			{
				const std::wstring wName = Platform::WStringFromString(pso.Name);
				const D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPso = RHI_Pipeline_DX12::GetGraphicsPipelineDesc(pso);

				ID3D12PipelineState* pipelineState = nullptr;
				m_library->LoadGraphicsPipeline(wName.c_str(), &graphicsPso, IID_PPV_ARGS(&pipelineState));

				FAIL_ASSERT();
				return nullptr;
			}

			void RHI_PipelineCache_DX12::AddPSO(const PipelineStateObject& pso, const RHI_Pipeline* pipeline)
			{
				const std::wstring wName = Platform::WStringFromString(pso.Name);
				m_library->StorePipeline(wName.c_str(), static_cast<const RHI_Pipeline_DX12*>(pipeline)->GetPipeline());
			}
		}
	}
}

#endif