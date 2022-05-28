#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#include <map>
#include <array>

#include <d3d12.h>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RHI_Shader_DX12 : public RHI_Shader
			{
			public:
				virtual ~RHI_Shader_DX12() override { Destroy(); }

				IDxcBlob* GetStage(ShaderStageFlagBits stage);
				D3D12_INPUT_LAYOUT_DESC GetInputLayout() const { return m_inputLayout; }

			private:
				virtual void Create(RenderContext* context, ShaderDesc desc) override;
				virtual void Destroy() override;

				void CompileStage(ShaderStageFlagBits stage, std::wstring_view path, int moduleIndex);
				void CreateVertexInputLayout(const ShaderDesc& desc);

			private:
				D3D12_INPUT_LAYOUT_DESC m_inputLayout;
				std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElements;
				ShaderDesc m_shaderDesc;
				std::array<ComPtr<IDxcBlob>, 5> m_modules;
				RenderContext_DX12* m_context{ nullptr };
			};
		}
	}
}

#endif // if defined(IS_DX12_ENABLED)