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

			struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
			{
				explicit CD3DX12_ROOT_SIGNATURE_DESC(const D3D12_ROOT_SIGNATURE_DESC& o) noexcept :
					D3D12_ROOT_SIGNATURE_DESC(o)
				{}
				CD3DX12_ROOT_SIGNATURE_DESC(
					UINT numParameters,
					_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
					UINT numStaticSamplers = 0,
					_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
					D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
				{
					Init(numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
				}
				CD3DX12_ROOT_SIGNATURE_DESC() noexcept
				{
					Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);
				}

				inline void Init(
					UINT numParameters,
					_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
					UINT numStaticSamplers = 0,
					_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
					D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
				{
					Init(*this, numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags);
				}

				static inline void Init(
					_Out_ D3D12_ROOT_SIGNATURE_DESC& desc,
					UINT numParameters,
					_In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
					UINT numStaticSamplers = 0,
					_In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
					D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) noexcept
				{
					desc.NumParameters = numParameters;
					desc.pParameters = _pParameters;
					desc.NumStaticSamplers = numStaticSamplers;
					desc.pStaticSamplers = _pStaticSamplers;
					desc.Flags = flags;
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


				//if (m_rootSigs.find(psoHash) == m_rootSigs.end())
				//{
				//	ComPtr<ID3D12RootSignature> rootSignature;
				//	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
				//	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

				//	ComPtr<ID3DBlob> signature;
				//	ComPtr<ID3DBlob> error;
				//	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
				//	ThrowIfFailed(m_context->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
				//	m_rootSigs[psoHash] = rootSignature;
				//}

				RHI_Shader_DX12* shader = dynamic_cast<RHI_Shader_DX12*>(pso.Shader);


				m_context->GetDescriptorLayoutManager().GetLayout(0, {});

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
				psoDesc.pRootSignature = m_rootSigs[psoHash].Get();
				psoDesc.VS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Vertex));
				psoDesc.HS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_TessControl));
				psoDesc.DS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_TessEval));
				psoDesc.GS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Geometry));
				psoDesc.PS = CD3DX12_SHADER_BYTECODE(shader->GetStage(ShaderStageFlagBits::ShaderStage_Pixel));
				psoDesc.RasterizerState = rasterizerState;
				psoDesc.BlendState = blendState;
				psoDesc.DepthStencilState.DepthEnable = FALSE;
				psoDesc.DepthStencilState.StencilEnable = FALSE;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.SampleDesc.Count = 1;

				ComPtr<ID3D12PipelineState> pipeline;
				ThrowIfFailed(m_context->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline)));
				m_pipelines[psoHash] = pipeline;
				return pipeline.Get();
			}

			ID3D12RootSignature* PipelineStateObjectManager_DX12::GetRootSignature(PipelineStateObject pso)
			{
				const u64 psoHash = pso.GetHash();
				auto itr = m_rootSigs.find(psoHash);
				if (itr != m_rootSigs.end())
				{
					return itr->second.Get();
				}
				return nullptr;
			}

			void PipelineStateObjectManager_DX12::Destroy()
			{
				m_pipelines.clear();
				m_rootSigs.clear();
			}
		}
	}
}