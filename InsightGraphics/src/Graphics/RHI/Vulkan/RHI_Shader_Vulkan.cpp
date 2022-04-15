#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Core/Logger.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <fstream>

// PDB not found for glslang
#pragma warning( disable : 4099 )

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			int RHI_Shader_Vulkan::s_gslangInit = 0;

			void InitResources(TBuiltInResource& Resources) 
			 {
				Resources.maxLights = 32;
				Resources.maxClipPlanes = 6;
				Resources.maxTextureUnits = 32;
				Resources.maxTextureCoords = 32;
				Resources.maxVertexAttribs = 64;
				Resources.maxVertexUniformComponents = 4096;
				Resources.maxVaryingFloats = 64;
				Resources.maxVertexTextureImageUnits = 32;
				Resources.maxCombinedTextureImageUnits = 80;
				Resources.maxTextureImageUnits = 32;
				Resources.maxFragmentUniformComponents = 4096;
				Resources.maxDrawBuffers = 32;
				Resources.maxVertexUniformVectors = 128;
				Resources.maxVaryingVectors = 8;
				Resources.maxFragmentUniformVectors = 16;
				Resources.maxVertexOutputVectors = 16;
				Resources.maxFragmentInputVectors = 15;
				Resources.minProgramTexelOffset = -8;
				Resources.maxProgramTexelOffset = 7;
				Resources.maxClipDistances = 8;
				Resources.maxComputeWorkGroupCountX = 65535;
				Resources.maxComputeWorkGroupCountY = 65535;
				Resources.maxComputeWorkGroupCountZ = 65535;
				Resources.maxComputeWorkGroupSizeX = 1024;
				Resources.maxComputeWorkGroupSizeY = 1024;
				Resources.maxComputeWorkGroupSizeZ = 64;
				Resources.maxComputeUniformComponents = 1024;
				Resources.maxComputeTextureImageUnits = 16;
				Resources.maxComputeImageUniforms = 8;
				Resources.maxComputeAtomicCounters = 8;
				Resources.maxComputeAtomicCounterBuffers = 1;
				Resources.maxVaryingComponents = 60;
				Resources.maxVertexOutputComponents = 64;
				Resources.maxGeometryInputComponents = 64;
				Resources.maxGeometryOutputComponents = 128;
				Resources.maxFragmentInputComponents = 128;
				Resources.maxImageUnits = 8;
				Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
				Resources.maxCombinedShaderOutputResources = 8;
				Resources.maxImageSamples = 0;
				Resources.maxVertexImageUniforms = 0;
				Resources.maxTessControlImageUniforms = 0;
				Resources.maxTessEvaluationImageUniforms = 0;
				Resources.maxGeometryImageUniforms = 0;
				Resources.maxFragmentImageUniforms = 8;
				Resources.maxCombinedImageUniforms = 8;
				Resources.maxGeometryTextureImageUnits = 16;
				Resources.maxGeometryOutputVertices = 256;
				Resources.maxGeometryTotalOutputComponents = 1024;
				Resources.maxGeometryUniformComponents = 1024;
				Resources.maxGeometryVaryingComponents = 64;
				Resources.maxTessControlInputComponents = 128;
				Resources.maxTessControlOutputComponents = 128;
				Resources.maxTessControlTextureImageUnits = 16;
				Resources.maxTessControlUniformComponents = 1024;
				Resources.maxTessControlTotalOutputComponents = 4096;
				Resources.maxTessEvaluationInputComponents = 128;
				Resources.maxTessEvaluationOutputComponents = 128;
				Resources.maxTessEvaluationTextureImageUnits = 16;
				Resources.maxTessEvaluationUniformComponents = 1024;
				Resources.maxTessPatchComponents = 120;
				Resources.maxPatchVertices = 32;
				Resources.maxTessGenLevel = 64;
				Resources.maxViewports = 16;
				Resources.maxVertexAtomicCounters = 0;
				Resources.maxTessControlAtomicCounters = 0;
				Resources.maxTessEvaluationAtomicCounters = 0;
				Resources.maxGeometryAtomicCounters = 0;
				Resources.maxFragmentAtomicCounters = 8;
				Resources.maxCombinedAtomicCounters = 8;
				Resources.maxAtomicCounterBindings = 1;
				Resources.maxVertexAtomicCounterBuffers = 0;
				Resources.maxTessControlAtomicCounterBuffers = 0;
				Resources.maxTessEvaluationAtomicCounterBuffers = 0;
				Resources.maxGeometryAtomicCounterBuffers = 0;
				Resources.maxFragmentAtomicCounterBuffers = 1;
				Resources.maxCombinedAtomicCounterBuffers = 1;
				Resources.maxAtomicCounterBufferSize = 16384;
				Resources.maxTransformFeedbackBuffers = 4;
				Resources.maxTransformFeedbackInterleavedComponents = 64;
				Resources.maxCullDistances = 8;
				Resources.maxCombinedClipAndCullDistances = 8;
				Resources.maxSamples = 4;
				Resources.maxMeshOutputVerticesNV = 256;
				Resources.maxMeshOutputPrimitivesNV = 512;
				Resources.maxMeshWorkGroupSizeX_NV = 32;
				Resources.maxMeshWorkGroupSizeY_NV = 1;
				Resources.maxMeshWorkGroupSizeZ_NV = 1;
				Resources.maxTaskWorkGroupSizeX_NV = 32;
				Resources.maxTaskWorkGroupSizeY_NV = 1;
				Resources.maxTaskWorkGroupSizeZ_NV = 1;
				Resources.maxMeshViewCountNV = 4;
				Resources.limits.nonInductiveForLoops = 1;
				Resources.limits.whileLoops = 1;
				Resources.limits.doWhileLoops = 1;
				Resources.limits.generalUniformIndexing = 1;
				Resources.limits.generalAttributeMatrixVectorIndexing = 1;
				Resources.limits.generalVaryingIndexing = 1;
				Resources.limits.generalSamplerIndexing = 1;
				Resources.limits.generalVariableIndexing = 1;
				Resources.limits.generalConstantMatrixVectorIndexing = 1;
			}

			EShLanguage FindLanguage(const vk::ShaderStageFlagBits shader_type) 
			 {
				switch (shader_type) {
				case vk::ShaderStageFlagBits::eVertex:
					return EShLangVertex;
				case vk::ShaderStageFlagBits::eTessellationControl:
					return EShLangTessControl;
				case vk::ShaderStageFlagBits::eTessellationEvaluation:
					return EShLangTessEvaluation;
				case vk::ShaderStageFlagBits::eGeometry:
					return EShLangGeometry;
				case vk::ShaderStageFlagBits::eFragment:
					return EShLangFragment;
				case vk::ShaderStageFlagBits::eCompute:
					return EShLangCompute;
				default:
					return EShLangVertex;
				}
			}

			void RHI_Shader_Vulkan::Create(RenderContext* context, ShaderDesc desc)
			{
				m_context = dynamic_cast<RenderContext_Vulkan*>(context);

				if (!desc.VertexFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eVertex, desc.VertexFilePath, 0); }
				if (!desc.TesselationControlFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eTessellationControl, desc.TesselationControlFilePath, 1); }
				if (!desc.TesselationEvaluationVertexFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eTessellationEvaluation, desc.TesselationEvaluationVertexFilePath, 2); }
				if (!desc.GeoemtyFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eGeometry, desc.GeoemtyFilePath, 3); }
				if (!desc.PixelFilePath.empty()) { CompileStage(vk::ShaderStageFlagBits::eFragment, desc.PixelFilePath, 4); }

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

				--s_gslangInit;
				if (s_gslangInit == 0)
				{
					glslang::FinalizeProcess();
				}
			}

			void RHI_Shader_Vulkan::CompileStage(vk::ShaderStageFlagBits stageType, std::string_view path, int moduleIndex)
			{
				// Compile glsl to spirv.
				if (s_gslangInit == 0)
				{
					glslang::InitializeProcess();
				}
				++s_gslangInit;

				std::ifstream inFile;
				inFile.open(path.data());
				if (!inFile.is_open())
				{
					IS_CORE_WARN("[GPUShader_Vulkan::CompileStage] Unable to open file: {}.", path);
					return;
				}

				std::string	pShader;
				std::string line;
				while (std::getline(inFile, line))
				{
					pShader += line + '\n';
				}

				EShLanguage stage = FindLanguage(stageType);
				glslang::TShader shader(stage);
				glslang::TProgram program;
				const char* shaderStrings[1];
				TBuiltInResource Resources = {};
				InitResources(Resources);

				// Enable SPIR-V and Vulkan rules when parsing GLSL
				EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

				shaderStrings[0] = pShader.data();
				shader.setStrings(shaderStrings, 1);

				if (!shader.parse(&Resources, 100, false, messages))
				{
					IS_CORE_INFO("[GPUShader_Vulkan::CompileStage] Info log: {}.", shader.getInfoLog());
					IS_CORE_INFO("[GPUShader_Vulkan::CompileStage] Info debug log: {}.", shader.getInfoDebugLog());
					return;  // something didn't work
				}

				program.addShader(&shader);

				if (!program.link(messages))
				{
					IS_CORE_INFO("[GPUShader_Vulkan::CompileStage] Info log: {}.", shader.getInfoLog());
					IS_CORE_INFO("[GPUShader_Vulkan::CompileStage] Info debug log: {}.", shader.getInfoDebugLog());
					return;
				}

				std::vector<u32> spirv;
				glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

				vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo({}, spirv);

				if (m_modules[moduleIndex])
				{
					m_context->GetDevice().destroyShaderModule(m_modules[moduleIndex]);
				}
				m_modules[moduleIndex] = m_context->GetDevice().createShaderModule(createInfo);
			}

			void RHI_Shader_Vulkan::CreateVertexInputLayout(const ShaderDesc& desc)
			{
				m_vertexInputLayout = {};
				int stride = 0;
				for (const auto& input : desc.VertexLayout)
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