#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Pipeline_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"

#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

#include <array>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			RHI_Pipeline_DX12::~RHI_Pipeline_DX12()
			{
				Release();
			}

			ID3D12PipelineState* RHI_Pipeline_DX12::GetPipeline() const
			{
				return m_pipeline;
			}

			void RHI_Pipeline_DX12::Create(RenderContext* context, PipelineStateObject pso)
			{
				m_context = static_cast<RenderContext_DX12*>(context);
				
				RHI_PipelineLayout_DX12* rootSignature = static_cast<RHI_PipelineLayout_DX12*>(m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso));

				RHI_Shader_DX12* vertexShader = static_cast<RHI_Shader_DX12*>(pso.Shader);
				D3D12_INPUT_LAYOUT_DESC inputElementDesc = vertexShader->GetInputLayout();

				D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
				inputLayoutDesc.pInputElementDescs = inputElementDesc.pInputElementDescs;
				inputLayoutDesc.NumElements = inputElementDesc.NumElements;

				CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
				depthStencilDesc.DepthEnable = pso.DepthTest;
				depthStencilDesc.DepthWriteMask = pso.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
				depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
				depthStencilDesc.StencilEnable = FALSE;

				std::array<CD3DX12_SHADER_BYTECODE, ShaderStageCount> shaderFuncNames;
				for (int i = 0; i < ShaderStageCount; ++i)
				{

				}

				// Describe and create the PSO for rendering the scene.
				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = inputLayoutDesc;
				psoDesc.pRootSignature = rootSignature->GetRootSignature();
				psoDesc.VS = shaderFuncNames.at(0);
				psoDesc.PS = shaderFuncNames.at(5);
				psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
				psoDesc.DepthStencilState = depthStencilDesc;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = PrimitiveTopologyTypeToDX12(pso.PrimitiveTopologyType);
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.SampleDesc.Count = 1;

				ThrowIfFailed(m_context->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline)));
				SetName(pso.Name + "_Pipeline");
			}

			void RHI_Pipeline_DX12::Release()
			{
				if (m_pipeline != nullptr)
				{
					m_pipeline->Release();
					m_pipeline = nullptr;
				}
			}

			bool RHI_Pipeline_DX12::ValidResource()
			{
				return  m_pipeline != nullptr;
			}

			void RHI_Pipeline_DX12::SetName(std::string name)
			{
				if (m_pipeline)
				{
					m_context->SetObjectName(name, m_pipeline);
				}
			}
		}
	}
}

#endif // IS_VULKAN_ENABLED