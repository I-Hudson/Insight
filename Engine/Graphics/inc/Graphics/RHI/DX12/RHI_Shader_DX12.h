#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Shader.h"
#include <d3d12.h>
#include <map>
#include <array>

#include "dxcapi.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			class RenderContext_DX12;

			class RHI_Shader_DX12 : public RHI_Shader
			{
			public:
				virtual ~RHI_Shader_DX12() override { Destroy(); }

				IDxcBlob* GetStage(ShaderStageFlagBits stage);
				D3D12_INPUT_LAYOUT_DESC GetInputLayout() const { return m_inputLayout; }

				// RHI_Resource - Begin
				virtual void Release() override { Destroy(); }
				virtual bool ValidResource() override { return m_compiled; }
				virtual void SetName(std::string name) { }
				// RHI_Resource - End

			private:
				virtual void Create(RenderContext* context, ShaderDesc desc) override;
				virtual void Destroy() override;

				void CompileStage(ShaderStageFlagBits stage, std::string_view path, int moduleIndex);
				void CompileStage(ShaderStageFlagBits stage, const std::vector<Byte>& shaderData, int moduleIndex);
				void CreateVertexInputLayout(const ShaderDesc& desc);

			private:
				D3D12_INPUT_LAYOUT_DESC m_inputLayout;
				std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElements;
				ShaderDesc m_shaderDesc;
				std::array<IDxcBlob*, ShaderStageCount> m_modules;
				RenderContext_DX12* m_context{ nullptr };
			};
		}
	}
}

#endif /// if defined(IS_DX12_ENABLED)