#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/DX12/PipelineStateObjectManager_DX12.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			struct CD3DX12_SHADER_BYTECODE : public D3D12_SHADER_BYTECODE
			{
				CD3DX12_SHADER_BYTECODE() = default;
				explicit CD3DX12_SHADER_BYTECODE(const D3D12_SHADER_BYTECODE& o) noexcept :
					D3D12_SHADER_BYTECODE(o)
				{}
				CD3DX12_SHADER_BYTECODE(
					_In_ IDxcBlob* pShaderBlob) noexcept
				{
					if (pShaderBlob)
					{
						pShaderBytecode = pShaderBlob->GetBufferPointer();
						BytecodeLength = pShaderBlob->GetBufferSize();
					}
					else
					{
						pShaderBytecode = nullptr;
						BytecodeLength = 0;
					}
				}
				CD3DX12_SHADER_BYTECODE(
					const void* _pShaderBytecode,
					SIZE_T bytecodeLength) noexcept
				{
					pShaderBytecode = _pShaderBytecode;
					BytecodeLength = bytecodeLength;
				}
			};

			PipelineStateObjectManager_DX12::PipelineStateObjectManager_DX12()
			{
			}

			PipelineStateObjectManager_DX12::~PipelineStateObjectManager_DX12()
			{
			}

			ID3D12PipelineState* PipelineStateObjectManager_DX12::GetOrCreatePSO(PipelineStateObject pso)
			{
				assert(m_context != nullptr);

				const u64 psoHash = pso.GetHash();
				auto itr = m_pipelines.find(psoHash);
				if (itr != m_pipelines.end())
				{
					return itr->second.Get();
				}

				RHI_Shader_DX12* shader = dynamic_cast<RHI_Shader_DX12*>(pso.Shader);

				RHI_DescriptorLayout_DX12* layout = dynamic_cast<RHI_DescriptorLayout_DX12*>(m_context->GetDescriptorLayoutManager().GetLayout(0, shader->GetDescriptors()));

				D3D12_RASTERIZER_DESC rasterizerState;
				rasterizerState.FillMode = PolygonModeToDX12(pso.PolygonMode);
				rasterizerState.CullMode = CullModeToDX12(pso.CullMode);
				rasterizerState.FrontCounterClockwise = pso.FrontFace == FrontFace::CounterClockwise ? true : false;
				rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
				rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
				rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
				rasterizerState.DepthClipEnable = TRUE;
				rasterizerState.MultisampleEnable = FALSE;
				rasterizerState.AntialiasedLineEnable = FALSE;
				rasterizerState.ForcedSampleCount = 0;
				rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

				D3D12_BLEND_DESC blendState;
				blendState.AlphaToCoverageEnable = FALSE;
				blendState.IndependentBlendEnable = FALSE;
				const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
				{
					FALSE,FALSE,
					D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
					D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
					D3D12_LOGIC_OP_NOOP,
					D3D12_COLOR_WRITE_ENABLE_ALL,
				};
				for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				{
					blendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
				}

				// Describe and create the graphics pipeline state object (PSO).
				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.InputLayout = shader->GetInputLayout();
				psoDesc.pRootSignature = layout->GetRootSignature();
				psoDesc.VS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Vertex));
				psoDesc.HS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_TessControl));
				psoDesc.DS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_TessEval));
				psoDesc.GS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Geometry));
				psoDesc.PS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Pixel));
				psoDesc.RasterizerState = rasterizerState;
				psoDesc.BlendState = blendState;
				psoDesc.DepthStencilState.DepthEnable = pso.DepthTest;
				psoDesc.DepthStencilState.DepthWriteMask = pso.DepthTest ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = // a stencil operation structure, again does not really matter since stencil testing is turned off
				{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
				psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
				psoDesc.DepthStencilState.BackFace = defaultStencilOp;
				psoDesc.DepthStencilState.StencilEnable = FALSE;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
				psoDesc.SampleDesc.Count = 1;

				ComPtr<ID3D12PipelineState> pipeline;
				ThrowIfFailed(m_context->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline)));
				if (lstrcmpW(pso.Name, L""))
				{
					pipeline->SetName(pso.Name);
				}
				m_pipelines[psoHash] = pipeline;
				return pipeline.Get();
			}

			void PipelineStateObjectManager_DX12::Destroy()
			{
				m_pipelines.clear();
			}
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)