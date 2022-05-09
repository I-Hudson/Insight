#include "Graphics/RHI/DX12/RHI_Shader_DX12.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/DX12/DX12Utils.h"

#include <bit>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::DX12
		{
			IDxcBlob* RHI_Shader_DX12::GetStage(ShaderStageFlagBits stage)
			{
				int index = BitFlagsToIndex(stage);
				return m_modules.at(index).Get();
			}

			void RHI_Shader_DX12::Create(RenderContext* context, ShaderDesc desc)
			{
				m_context = dynamic_cast<RenderContext_DX12*>(context);
				m_shaderDesc = desc;

				if (!desc.VertexFilePath.empty()) { CompileStage(ShaderStageFlagBits::ShaderStage_Vertex, desc.VertexFilePath, 0); }
				if (!desc.TesselationControlFilePath.empty()) { CompileStage(ShaderStageFlagBits::ShaderStage_TessControl, desc.TesselationControlFilePath, 1); }
				if (!desc.TesselationEvaluationVertexFilePath.empty()) { CompileStage(ShaderStageFlagBits::ShaderStage_TessEval, desc.TesselationEvaluationVertexFilePath, 2); }
				if (!desc.GeoemtyFilePath.empty()) { CompileStage(ShaderStageFlagBits::ShaderStage_Geometry, desc.GeoemtyFilePath, 3); }
				if (!desc.PixelFilePath.empty()) { CompileStage(ShaderStageFlagBits::ShaderStage_Pixel, desc.PixelFilePath, 4); }

				CreateVertexInputLayout(m_shaderDesc);
			}

			void RHI_Shader_DX12::Destroy()
			{
				for (ComPtr<IDxcBlob>& mod : m_modules)
				{
					if (mod)
					{
						mod.Reset();
						mod = nullptr;
					}
				}
			}

			void RHI_Shader_DX12::CompileStage(ShaderStageFlagBits stage, std::wstring_view path, int moduleIndex)
			{
				ShaderCompiler compiler;
				ComPtr<IDxcBlob> code = compiler.Compile(stage, path, ShaderCompilerLanguage::Hlsl);
				compiler.GetDescriptors(stage, m_descriptors);
				m_modules[moduleIndex] = code;
			}

			void RHI_Shader_DX12::CreateVertexInputLayout(const ShaderDesc& desc)
			{
				m_inputElements = {};
				int stride = 0;
				for (size_t i = 0; i < desc.VertexLayout.size(); ++i)
				{
					const ShaderInputLayout& inputLayout = desc.VertexLayout.at(i);
					D3D12_INPUT_ELEMENT_DESC input;
					input.SemanticName = inputLayout.Name.c_str();
					input.SemanticIndex = 0;
					input.Format = PixelFormatToDX12(inputLayout.Format);
					input.InputSlot = 0;
					input.AlignedByteOffset = stride;
					input.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
					input.InstanceDataStepRate = 0;

					stride += PixelFormatExtensions::SizeInBytes(inputLayout.Format);
					m_inputElements.push_back(input);
				}

				m_inputLayout.pInputElementDescs = m_inputElements.data();
				m_inputLayout.NumElements = static_cast<UINT>(m_inputElements.size());
			}
		}
	}
}