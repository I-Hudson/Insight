#include "ispch.h"
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"
#include "VulkanFrameBuffer.h"
#include "Insight/ShaderParser/ShaderParser.h"

#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "StandAlone/DirStackFileIncluder.h"

namespace vks
{
	std::string GetSuffix(const std::string& name)
	{
		const size_t pos = name.rfind('.');
		return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
	}

	std::string GetShaderFileName(const std::string& fileName)
	{
		return fileName.substr(fileName.find_last_of('/'), fileName.find_last_of('.'));
	}

	EShLanguage GetShaderStage(const std::string& stage)
	{
		if (stage == "vert") {
			return EShLangVertex;
		}
		else if (stage == "tesc") {
			return EShLangTessControl;
		}
		else if (stage == "tese") {
			return EShLangTessEvaluation;
		}
		else if (stage == "geom") {
			return EShLangGeometry;
		}
		else if (stage == "frag") {
			return EShLangFragment;
		}
		else if (stage == "comp") {
			return EShLangCompute;
		}
		else {
			assert(0 && "Unknown shader stage");
			return EShLangCount;
		}
	}

	const TBuiltInResource DefaultTBuiltInResource = {
		/* .MaxLights = */ 32,
		/* .MaxClipPlanes = */ 6,
		/* .MaxTextureUnits = */ 32,
		/* .MaxTextureCoords = */ 32,
		/* .MaxVertexAttribs = */ 64,
		/* .MaxVertexUniformComponents = */ 4096,
		/* .MaxVaryingFloats = */ 64,
		/* .MaxVertexTextureImageUnits = */ 32,
		/* .MaxCombinedTextureImageUnits = */ 80,
		/* .MaxTextureImageUnits = */ 32,
		/* .MaxFragmentUniformComponents = */ 4096,
		/* .MaxDrawBuffers = */ 32,
		/* .MaxVertexUniformVectors = */ 128,
		/* .MaxVaryingVectors = */ 8,
		/* .MaxFragmentUniformVectors = */ 16,
		/* .MaxVertexOutputVectors = */ 16,
		/* .MaxFragmentInputVectors = */ 15,
		/* .MinProgramTexelOffset = */ -8,
		/* .MaxProgramTexelOffset = */ 7,
		/* .MaxClipDistances = */ 8,
		/* .MaxComputeWorkGroupCountX = */ 65535,
		/* .MaxComputeWorkGroupCountY = */ 65535,
		/* .MaxComputeWorkGroupCountZ = */ 65535,
		/* .MaxComputeWorkGroupSizeX = */ 1024,
		/* .MaxComputeWorkGroupSizeY = */ 1024,
		/* .MaxComputeWorkGroupSizeZ = */ 64,
		/* .MaxComputeUniformComponents = */ 1024,
		/* .MaxComputeTextureImageUnits = */ 16,
		/* .MaxComputeImageUniforms = */ 8,
		/* .MaxComputeAtomicCounters = */ 8,
		/* .MaxComputeAtomicCounterBuffers = */ 1,
		/* .MaxVaryingComponents = */ 60,
		/* .MaxVertexOutputComponents = */ 64,
		/* .MaxGeometryInputComponents = */ 64,
		/* .MaxGeometryOutputComponents = */ 128,
		/* .MaxFragmentInputComponents = */ 128,
		/* .MaxImageUnits = */ 8,
		/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
		/* .MaxCombinedShaderOutputResources = */ 8,
		/* .MaxImageSamples = */ 0,
		/* .MaxVertexImageUniforms = */ 0,
		/* .MaxTessControlImageUniforms = */ 0,
		/* .MaxTessEvaluationImageUniforms = */ 0,
		/* .MaxGeometryImageUniforms = */ 0,
		/* .MaxFragmentImageUniforms = */ 8,
		/* .MaxCombinedImageUniforms = */ 8,
		/* .MaxGeometryTextureImageUnits = */ 16,
		/* .MaxGeometryOutputVertices = */ 256,
		/* .MaxGeometryTotalOutputComponents = */ 1024,
		/* .MaxGeometryUniformComponents = */ 1024,
		/* .MaxGeometryVaryingComponents = */ 64,
		/* .MaxTessControlInputComponents = */ 128,
		/* .MaxTessControlOutputComponents = */ 128,
		/* .MaxTessControlTextureImageUnits = */ 16,
		/* .MaxTessControlUniformComponents = */ 1024,
		/* .MaxTessControlTotalOutputComponents = */ 4096,
		/* .MaxTessEvaluationInputComponents = */ 128,
		/* .MaxTessEvaluationOutputComponents = */ 128,
		/* .MaxTessEvaluationTextureImageUnits = */ 16,
		/* .MaxTessEvaluationUniformComponents = */ 1024,
		/* .MaxTessPatchComponents = */ 120,
		/* .MaxPatchVertices = */ 32,
		/* .MaxTessGenLevel = */ 64,
		/* .MaxViewports = */ 16,
		/* .MaxVertexAtomicCounters = */ 0,
		/* .MaxTessControlAtomicCounters = */ 0,
		/* .MaxTessEvaluationAtomicCounters = */ 0,
		/* .MaxGeometryAtomicCounters = */ 0,
		/* .MaxFragmentAtomicCounters = */ 8,
		/* .MaxCombinedAtomicCounters = */ 8,
		/* .MaxAtomicCounterBindings = */ 1,
		/* .MaxVertexAtomicCounterBuffers = */ 0,
		/* .MaxTessControlAtomicCounterBuffers = */ 0,
		/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
		/* .MaxGeometryAtomicCounterBuffers = */ 0,
		/* .MaxFragmentAtomicCounterBuffers = */ 1,
		/* .MaxCombinedAtomicCounterBuffers = */ 1,
		/* .MaxAtomicCounterBufferSize = */ 16384,
		/* .MaxTransformFeedbackBuffers = */ 4,
		/* .MaxTransformFeedbackInterleavedComponents = */ 64,
		/* .MaxCullDistances = */ 8,
		/* .MaxCombinedClipAndCullDistances = */ 8,
		/* .MaxSamples = */ 4,
		/* .maxMeshOutputVerticesNV = */ 0,
		/* .maxMeshOutputPrimitivesNV = */  0,
		/* .maxMeshWorkGroupSizeX_NV = */  0,
		/* .maxMeshWorkGroupSizeY_NV = */ 0,
		/* .maxMeshWorkGroupSizeZ_NV = */ 0,
		/* .maxTaskWorkGroupSizeX_NV = */ 0,
		/* .maxTaskWorkGroupSizeY_NV = */ 0,
		/* .maxTaskWorkGroupSizeZ_NV = */ 0,
		/* .maxMeshViewCountNV = */ 0,
		/* .maxDualSourceDrawBuffersEXT = */ 0,
		/* .limits = */ {
			/* .nonInductiveForLoops = */ 1,
			/* .whileLoops = */ 1,
			/* .doWhileLoops = */ 1,
			/* .generalUniformIndexing = */ 1,
			/* .generalAttributeMatrixVectorIndexing = */ 1,
			/* .generalVaryingIndexing = */ 1,
			/* .generalSamplerIndexing = */ 1,
			/* .generalVariableIndexing = */ 1,
			/* .generalConstantMatrixVectorIndexing = */ 1,
		}
	};

	VulkanPipeline::VulkanPipeline()
	{
	}

	VulkanPipeline::VulkanPipeline(VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, const RenderPassInfo& renderPassInfo)
		: m_vulkanDevice(device)
	{

		std::vector<Insight::ParsedShadeData> shaderData;
		for (size_t i = 0; i < shaders.size(); ++i)
		{
			shaderData.push_back(Insight::ShaderParser::ParseShader(shaders[i]));
		}

		CreateDescriptorSetLayout(device, shaders, shaderData);
		CreatePipelineLayout(device, shaders, shaderData);
		CreatePipeline(device, shaders, renderPass, shaderData, renderPassInfo);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		for (auto& set : m_descriptorLayouts)
		{
			vkDestroyDescriptorSetLayout(*m_vulkanDevice, set.second, nullptr);
		}
		vkDestroyPipelineLayout(*m_vulkanDevice, m_pipelineLayout, nullptr);
		vkDestroyPipeline(*m_vulkanDevice, m_pipeline, nullptr);
	}

	void VulkanPipeline::Create(VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, std::vector<Insight::ParsedShadeData>& shaderData, const RenderPassInfo& renderPassInfo)
	{
		m_vulkanDevice = device;

		CreateDescriptorSetLayout(device, shaders, shaderData);
		CreatePipelineLayout(device, shaders, shaderData);
		CreatePipeline(device, shaders, renderPass, shaderData, renderPassInfo);
	}

	void VulkanPipeline::Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
	{
		vkCmdBindPipeline(commandBuffer, bindPoint, m_pipeline);
	}

	void VulkanPipeline::CreateDescriptorSetLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData)
	{
		std::set<int> setIndexs;

		for (auto& data : shaderData)
		{
			for (auto& buffer : data.UniformBlocks)
			{
				setIndexs.insert(buffer.Set);
			}
		}

		//TODO Please clean this up it's horriable.
		for (auto& set : setIndexs)
		{
			std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			std::vector<VkDescriptorBindingFlagsEXT> bindFlag;
			for (auto& data : shaderData)
			{
				for (auto& buffer : data.UniformBlocks)
				{
					if (buffer.Set == set)
					{
						descriptorSetLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(buffer.GetVulkanType(), data.GetVulkanShaderStage(), buffer.Binding));
						bindFlag.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
					}
				}
			}

			// TODO: Depending on check from Vulkan Device. Do this.
			VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
			extendedInfo.pNext = nullptr;
			extendedInfo.bindingCount = static_cast<U32>(descriptorSetLayoutBindings.size());
			extendedInfo.pBindingFlags = bindFlag.data();

			auto createInfo = vks::initializers::descriptorSetLayoutCreateInfo(descriptorSetLayoutBindings);
			createInfo.pNext = &extendedInfo;
			ThrowIfFailed(vkCreateDescriptorSetLayout(*device, &createInfo, nullptr, &m_descriptorLayouts[set]));
		}
	}

	void VulkanPipeline::CreatePipelineLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData)
	{
		std::vector<VkPushConstantRange> pushConstants;

		for (auto& data : shaderData)
		{
			for (auto& buffer : data.PushConstants)
			{
				pushConstants.push_back(vks::initializers::pushConstantRange(data.GetVulkanShaderStage(), buffer.Size, buffer.Offset));
			}
		}

		std::vector<VkDescriptorSetLayout> setLayouts;
		for (auto& set : m_descriptorLayouts)
		{
			setLayouts.push_back(set.second);
		}
		ThrowIfFailed(vkCreatePipelineLayout(*device, &vks::initializers::pipelineLayoutCreateInfo(setLayouts, pushConstants), nullptr, &m_pipelineLayout));
	}

	void VulkanPipeline::CreatePipeline(vks::VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, std::vector<Insight::ParsedShadeData>& shaderData, const RenderPassInfo& renderPassInfo)
	{
		VkSpecializationMapEntry specializationEntry{};
		specializationEntry.constantID = 0;
		specializationEntry.offset = 0;
		specializationEntry.size = sizeof(uint32_t);

		VkSpecializationInfo specializationInfo;
		specializationInfo.mapEntryCount = 1;
		specializationInfo.pMapEntries = &specializationEntry;
		specializationInfo.dataSize = sizeof(renderPassInfo.SamplerCount);
		specializationInfo.pData = &renderPassInfo.SamplerCount;

		Insight::ParsedShadeData vertexShaderData;
		std::vector<std::vector<uint32_t>> compiledShaders;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (size_t i = 0; i < shaders.size(); ++i)
		{
			//TODO check for shader with spirv prefix. No need to compile shader if it's already done.
			
			compiledShaders.push_back(CompileGLSL(shaders[i]));
			shaderStages.push_back(loadShaderFromSPIRV(compiledShaders[i], *device, shaderData[i].GetVulkanShaderStage()));
			shaderStages[shaderStages.size() - 1].pSpecializationInfo = &specializationInfo;

			if (shaderData[i].GetVulkanShaderStage() == VK_SHADER_STAGE_VERTEX_BIT)
			{
				vertexShaderData = shaderData[i];
			}
		}
		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = vertexShaderData.GetVertexInputState();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);;
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE, renderPassInfo.ColorAttachmentCount);
		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(renderPassInfo.ColorAttachmentCount, blendAttachmentStates.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(static_cast<VkSampleCountFlagBits>(renderPassInfo.SamplerCount));
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

		if (vertexInputStateCreateInfo.vertexAttributeDescriptionCount == 0)
		{
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(m_pipelineLayout, renderPass);
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;

		ThrowIfFailed(vkCreateGraphicsPipelines(*device, device->GetPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_pipeline));

		for (auto shaderModule : shaderStages)
		{
			vkDestroyShaderModule(*device, shaderModule.module, nullptr);
		}
	}

	std::vector<uint32_t> VulkanPipeline::CompileGLSL(const std::string& fileName)
	{
		std::string glslRaw = loadShaderString(fileName);
		const char* glslRawC = glslRaw.c_str();

		EShLanguage ShaderType = GetShaderStage(GetSuffix(fileName));
		glslang::TShader Shader(ShaderType);
		Shader.setStrings(&glslRawC, 1);

		int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
		glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
		glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

		Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
		Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

		TBuiltInResource Resources;
		Resources = DefaultTBuiltInResource;
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
		const int DefaultVersion = 100;

		DirStackFileIncluder Includer;
		//Get Path of File
		Includer.pushExternalLocalDirectory(fileName);

		std::string PreprocessedGLSL;
		if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
		{
			IS_CORE_ERROR("GLSL preprocess Failed for: {0}.", fileName);
			IS_CORE_ERROR("{0}", Shader.getInfoLog());
			IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
		}

		if (!Shader.parse(&Resources, 100, false, messages))
		{
			IS_CORE_ERROR("GLSL parse Failed for: {0}.", fileName);
			IS_CORE_ERROR("{0}", Shader.getInfoLog());
			IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
		}

		glslang::TProgram Program;
		Program.addShader(&Shader);
		if (!Program.link(messages))
		{
			IS_CORE_ERROR("GLSL Linking Failed for: {0}.", fileName);
			IS_CORE_ERROR("{0}", Shader.getInfoLog());
			IS_CORE_ERROR("{0}", Shader.getInfoDebugLog());
		}

		std::vector<uint32_t> SpirV;
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger);

		return SpirV;
	}
}