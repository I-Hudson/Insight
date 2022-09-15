#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Core/Logger.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "dxc/dxcapi.h"
#include "spirv_reflect.h"

#include <fstream>

// PDB not found for glslang
#pragma warning( disable : 4099 )

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			vk::ShaderModule RHI_Shader_Vulkan::GetStage(ShaderStageFlagBits stage) const
			{
				int index = BitFlagsToIndex(stage);
				return m_modules.at(index);
			}

			std::wstring_view RHI_Shader_Vulkan::GetMainFuncName(ShaderStageFlagBits stage) const
			{
				int index = BitFlagsToIndex(stage);
				return m_mainFuncNames.at(index);
			}

			void RHI_Shader_Vulkan::Create(RenderContext* context, ShaderDesc desc)
			{
				m_context = static_cast<RenderContext_Vulkan*>(context);

				if (!desc.VertexFilePath.empty())						{ CompileStage(ShaderStageFlagBits::ShaderStage_Vertex, desc.VertexFilePath, 0); }
				if (!desc.TesselationControlFilePath.empty())			{ CompileStage(ShaderStageFlagBits::ShaderStage_TessControl, desc.TesselationControlFilePath, 1); }
				if (!desc.TesselationEvaluationVertexFilePath.empty())	{ CompileStage(ShaderStageFlagBits::ShaderStage_TessEval, desc.TesselationEvaluationVertexFilePath, 2); }
				if (!desc.GeoemtyFilePath.empty())						{ CompileStage(ShaderStageFlagBits::ShaderStage_Geometry, desc.GeoemtyFilePath, 3); }
				if (!desc.PixelFilePath.empty())						{ CompileStage(ShaderStageFlagBits::ShaderStage_Pixel, desc.PixelFilePath, 4); }

				CreateVertexInputLayout(desc);
			}

			void RHI_Shader_Vulkan::Destroy()
			{
				for (vk::ShaderModule& mod : m_modules)
				{
					if (mod)
					{
						m_context->GetDevice().destroyShaderModule(mod);
						mod = vk::ShaderModule(nullptr);
					}
				}
			}

			void RHI_Shader_Vulkan::CompileStage(ShaderStageFlagBits stage, std::wstring_view path, int moduleIndex)
			{
				ShaderCompiler compiler;
				ComPtr<IDxcBlob> code = compiler.Compile(stage, path, ShaderCompilerLanguage::Spirv);
				compiler.GetDescriptors(stage, m_descriptors, m_push_constant);
				if (!code)
				{
					return;
				}

				vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, {});
				createInfo.setCodeSize(code->GetBufferSize());
				createInfo.setPCode((u32*)code->GetBufferPointer());

				if (m_modules[moduleIndex])
				{
					m_context->GetDevice().destroyShaderModule(m_modules[moduleIndex]);
				}
				m_modules[moduleIndex] = m_context->GetDevice().createShaderModule(createInfo);
				m_mainFuncNames[moduleIndex] = compiler.StageToFuncName(stage);
				if (!m_modules[moduleIndex])
				{
					IS_CORE_ERROR("Shader compilation failed.");
					return;
				}
				m_compiled = true;
			}

			void RHI_Shader_Vulkan::CreateVertexInputLayout(const ShaderDesc& desc)
			{
				if (!desc.InputLayout.empty())
				{
					m_shaderInputLayout = desc.InputLayout;
				}
				else
				{
					ShaderCompiler compiler;
					compiler.Compile(ShaderStage_Vertex, desc.VertexFilePath, ShaderCompilerLanguage::Spirv);
					m_shaderInputLayout = compiler.GetInputLayout();
				}

				m_vertexInputLayout = {};
				int stride = 0;
				for (const auto& input : m_shaderInputLayout)
				{
					vk::VertexInputAttributeDescription attri = {};
					attri.location = input.Binding;
					attri.binding = 0;
					attri.format = PixelFormatToVulkan(input.Format);
					attri.offset = stride;
					stride += PixelFormatExtensions::SizeInBytes(input.Format);

					m_vertexInputLayout.Attributes.push_back(attri);
				}

				if (m_vertexInputLayout.Attributes.size() > 0)
				{
					vk::VertexInputBindingDescription inputDesc = {};
					inputDesc.binding = 0;
					inputDesc.stride = stride;
					inputDesc.inputRate = vk::VertexInputRate::eVertex;
					m_vertexInputLayout.Bindings.push_back(inputDesc);
					m_vertexInputLayout.CreateInfo = vk::PipelineVertexInputStateCreateInfo({}, m_vertexInputLayout.Bindings, m_vertexInputLayout.Attributes);
				}
			}
		}
	}
}

#pragma warning( default : 4099 )

#endif //#if defined(IS_VULKAN_ENABLED)