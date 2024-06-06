#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/RHI_Pipeline_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"

#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/RHI/DX12/RHI_Texture_DX12.h"
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

				ASSERT(pso.Shader);
				RHI_Shader_DX12* vertexShader = static_cast<RHI_Shader_DX12*>(pso.Shader);
				D3D12_INPUT_LAYOUT_DESC inputElementDesc = vertexShader->GetInputLayout();

				D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
				inputLayoutDesc.pInputElementDescs = inputElementDesc.pInputElementDescs;
				inputLayoutDesc.NumElements = inputElementDesc.NumElements;

				CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
				depthStencilDesc.DepthEnable = pso.DepthTest;
				depthStencilDesc.DepthWriteMask = pso.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
				depthStencilDesc.DepthFunc = CompareOpToDX12(pso.DepthCompareOp);
				depthStencilDesc.StencilEnable = FALSE;

				std::array<CD3DX12_SHADER_BYTECODE, ShaderStageCount> shaderFuncNames;
				for (int i = 0; i < ShaderStageCount; ++i)
				{
					IDxcBlob* blob = vertexShader->GetStage(static_cast<ShaderStageFlagBits>(1 << i));
					shaderFuncNames.at(i) = CD3DX12_SHADER_BYTECODE(nullptr, 0);
					if (blob)
					{
						shaderFuncNames.at(i) = CD3DX12_SHADER_BYTECODE(blob->GetBufferPointer(), blob->GetBufferSize());
					}
				}

				CD3DX12_RASTERIZER_DESC rasterizerState(
					PolygonModeToDX12(pso.PolygonMode),
					CullModeToDX12(pso.CullMode),
					pso.FrontFace == FrontFace::CounterClockwise ? true : false,
					static_cast<INT>(pso.DepthConstantBaisValue),
					D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
					pso.DepthSlopeBaisValue,
					TRUE,
					FALSE,
					FALSE,
					0,
					D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
				);

				CD3DX12_BLEND_DESC blendState(
					D3D12_DEFAULT
				);
				for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				{
					const D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc
					{
						pso.BlendEnable, FALSE,
						BlendFactorToDX12(pso.SrcColourBlendFactor), BlendFactorToDX12(pso.DstColourBlendFactor),
						BlendOpToDX12(pso.ColourBlendOp),
						BlendFactorToDX12(pso.SrcAplhaBlendFactor), BlendFactorToDX12(pso.DstAplhaBlendFactor),
						BlendOpToDX12(pso.AplhaBlendOp),
						D3D12_LOGIC_OP_NOOP,
						D3D12_COLOR_WRITE_ENABLE_ALL
					};
					blendState.RenderTarget[i] = renderTargetBlendDesc;
				}

				// Describe and create the PSO for rendering the scene.
				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = inputLayoutDesc;
				psoDesc.pRootSignature = rootSignature->GetRootSignature();
				psoDesc.VS = shaderFuncNames.at(0);
				psoDesc.PS = shaderFuncNames.at(4);
				psoDesc.HS = shaderFuncNames.at(1);
				psoDesc.DS = shaderFuncNames.at(2);
				psoDesc.GS = shaderFuncNames.at(3);
				psoDesc.RasterizerState = rasterizerState;
				psoDesc.BlendState = blendState;
				psoDesc.DepthStencilState = depthStencilDesc;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = PrimitiveTopologyTypeToDX12(pso.PrimitiveTopologyType);
				psoDesc.SampleDesc.Count = 1;

				if (pso.Swapchain)
				{
					pso.RenderTargets.at(0) = m_context->GetSwaphchainIamge();
				}

				u32 renderTargetCount = 0;
				for (size_t i = 0; i < pso.RenderTargetCount; ++i)
				{
					RHI_Texture* renderTarget = pso.RenderTargets.at(i);
					if (renderTarget)
					{
						RHI_Texture_DX12* renderTargetDX12 = static_cast<RHI_Texture_DX12*>(renderTarget);
						psoDesc.RTVFormats[i] = PixelFormatToDX12(renderTargetDX12->GetFormat());
						++renderTargetCount;
					}
				}
				psoDesc.NumRenderTargets = renderTargetCount;


				if (pso.DepthStencil)
				{
					RHI_Texture_DX12* depthTargetDX12 = static_cast<RHI_Texture_DX12*>(pso.DepthStencil);
					psoDesc.DSVFormat = PixelFormatToDX12(depthTargetDX12->GetFormat());
				}
				else if (pso.DepthStencilFormat != PixelFormat::Unknown)
				{
					psoDesc.DSVFormat = PixelFormatToDX12(pso.DepthStencilFormat);
				}

				ThrowIfFailed(m_context->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline)));
				SetName(pso.Name + "_GraphicsPipeline");
			}

			void RHI_Pipeline_DX12::Create(RenderContext* context, ComputePipelineStateObject pso)
			{
				m_context = static_cast<RenderContext_DX12*>(context);

				RHI_PipelineLayout_DX12* rootSignature = static_cast<RHI_PipelineLayout_DX12*>(m_context->GetPipelineLayoutManager().GetOrCreateLayout(pso));

				ASSERT(pso.Shader);
				RHI_Shader_DX12* vertexShader = static_cast<RHI_Shader_DX12*>(pso.Shader);
				IDxcBlob* blob = vertexShader->GetStage(ShaderStage_Compute);
				CD3DX12_SHADER_BYTECODE shaderByteCode(blob->GetBufferPointer(), blob->GetBufferSize());

				// Describe and create the PSO for compute.
				D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.pRootSignature = rootSignature->GetRootSignature();
				psoDesc.CS = shaderByteCode;

				ThrowIfFailed(m_context->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline)));
				SetName(pso.Name + "_ComputePipeline");
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