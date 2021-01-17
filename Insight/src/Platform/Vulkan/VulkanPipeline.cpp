#include "ispch.h"
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"
#include "VulkanFrameBuffer.h"
#include "Insight/ShaderParser/ShaderParser.h"

#include <shaderc/shaderc.hpp>

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

	shaderc_shader_kind  GetShaderStage(const std::string& stage)
	{
		if (stage == "vert")
		{
			return shaderc_shader_kind::shaderc_vertex_shader;
		}
		else if (stage == "tesc")
		{
			return shaderc_shader_kind::shaderc_tess_control_shader;
		}
		else if (stage == "tese")
		{
			return shaderc_shader_kind::shaderc_tess_evaluation_shader;
		}
		else if (stage == "geom")
		{
			return shaderc_shader_kind::shaderc_geometry_shader;
		}
		else if (stage == "frag")
		{
			return shaderc_shader_kind::shaderc_fragment_shader;
		}
		else if (stage == "comp")
		{
			return shaderc_shader_kind::shaderc_compute_shader;
		}
		else
		{
			assert(0 && "Unknown shader stage");
			return shaderc_shader_kind::shaderc_anyhit_shader;
		}
	}

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
			for (auto& data : shaderData)
			{
				for (auto& buffer : data.UniformBlocks)
				{
					if (buffer.Set == set)
					{
						descriptorSetLayoutBindings.push_back(vks::initializers::descriptorSetLayoutBinding(buffer.GetVulkanType(), data.GetVulkanShaderStage(), buffer.Binding));
					}
				}
			}

			auto createInfo = vks::initializers::descriptorSetLayoutCreateInfo(descriptorSetLayoutBindings);
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
		Insight::ParsedShadeData vertexShaderData;
		std::vector<std::vector<uint32_t>> compiledShaders;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		for (size_t i = 0; i < shaders.size(); ++i)
		{
			//TODO check for shader with spirv prefix. No need to compile shader if it's already done.
			
			compiledShaders.push_back(CompileGLSL(shaders[i]));
			shaderStages.push_back(loadShaderFromSPIRV(compiledShaders[i], *device, shaderData[i].GetVulkanShaderStage()));

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
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
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
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");
		if (false) options.SetOptimizationLevel(shaderc_optimization_level_size);

		auto source = vks::loadShaderString(fileName);

		shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, GetShaderStage(GetSuffix(fileName)), GetShaderFileName(fileName).c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
			return std::vector<uint32_t>();
		}

		return { module.cbegin(), module.cend() };

	}
}