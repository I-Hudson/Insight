#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Core/Logger.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "dxc/dxcapi.h"
#include "spirv_reflect.h"

#include <fstream>

/// PDB not found for glslang
#pragma warning( disable : 4099 )

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			VkShaderModule RHI_Shader_Vulkan::GetStage(ShaderStageFlagBits stage) const
			{
				int index = BitFlagsToIndex(stage);
				return m_modules.at(index);
			}

			std::string_view RHI_Shader_Vulkan::GetMainFuncName(ShaderStageFlagBits stage) const
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
				for (VkShaderModule& mod : m_modules)
				{
					if (mod)
					{
						vkDestroyShaderModule(m_context->GetDevice(), mod, nullptr);
						mod = nullptr;
					}
				}
			}

			void RHI_Shader_Vulkan::CompileStage(ShaderStageFlagBits stage, std::string_view path, int moduleIndex)
			{
				ShaderCompiler compiler;
				IDxcBlob* code = compiler.Compile(stage, path, ShaderCompilerLanguage::Spirv);
				if (!code)
				{
					return;
				}

				compiler.GetDescriptorSets(stage, m_descriptor_sets, m_push_constant);

				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code->GetBufferSize();
				createInfo.pCode = static_cast<u32*>(code->GetBufferPointer());

				if (m_modules[moduleIndex])
				{
					vkDestroyShaderModule(m_context->GetDevice(), m_modules[moduleIndex], nullptr);
					m_modules[moduleIndex] = nullptr;
				}

				VkShaderModule shaderModule = nullptr;
				ThrowIfFailed(vkCreateShaderModule(m_context->GetDevice(), &createInfo, nullptr, &shaderModule));
				m_modules[moduleIndex] = shaderModule;

				m_mainFuncNames[moduleIndex] = compiler.StageToFuncName(stage);
				if (!m_modules[moduleIndex])
				{
					code->Release();
					IS_CORE_ERROR("Shader compilation failed.");
					return;
				}
				code->Release();
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
					VkVertexInputAttributeDescription attri = {};
					attri.location = input.Binding;
					attri.binding = 0;
					attri.format = PixelFormatToVulkan(input.Format);
					attri.offset = stride;
					stride += PixelFormatExtensions::SizeInBytes(input.Format);

					m_vertexInputLayout.Attributes.push_back(attri);
				}

				if (m_vertexInputLayout.Attributes.size() > 0)
				{
					VkVertexInputBindingDescription inputDesc = {};
					inputDesc.binding = 0;
					inputDesc.stride = stride;
					inputDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
					m_vertexInputLayout.Bindings.push_back(inputDesc);
				}

				VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
				pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = m_vertexInputLayout.Bindings.data();
				pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<u32>(m_vertexInputLayout.Bindings.size());
				pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = m_vertexInputLayout.Attributes.data();
				pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<u32>(m_vertexInputLayout.Attributes.size());

				m_vertexInputLayout.CreateInfo = pipelineVertexInputStateCreateInfo;
			}
		}
	}
}

#pragma warning( default : 4099 )

#endif ///#if defined(IS_VULKAN_ENABLED)