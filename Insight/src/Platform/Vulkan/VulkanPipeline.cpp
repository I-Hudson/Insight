#include "ispch.h"
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "Insight/ShaderParser/ShaderParser.h"
#include <shaderc\shaderc.hpp>

namespace vks
{
	std::string GetSuffix(const std::string& name)
	{
		const size_t pos = name.rfind('.');
		return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
	}

	shaderc_shader_kind GetShaderStage(const std::string& stage)
	{
		if (stage == "vert")
		{
			return shaderc_shader_kind::shaderc_glsl_vertex_shader;
		}
		else if (stage == "tesc")
		{
			return shaderc_shader_kind::shaderc_glsl_tess_control_shader;
		}
		else if (stage == "tese")
		{
			return shaderc_shader_kind::shaderc_glsl_tess_evaluation_shader;
		}
		else if (stage == "geom")
		{
			return shaderc_shader_kind::shaderc_glsl_geometry_shader;
		}
		else if (stage == "frag")
		{
			return shaderc_shader_kind::shaderc_glsl_fragment_shader;
		}
		else if (stage == "comp")
		{
			return shaderc_shader_kind::shaderc_glsl_compute_shader;
		}
		return shaderc_glsl_vertex_shader;
	}

	VulkanPipeline::VulkanPipeline(VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor)
		: m_vulkanDevice(device)
	{
		CreateDescriptorSetLayout(device, shaders);
		CreatePipelineLayout(device, shaders);
		CreatePipeline(device, shaders, renderPass, viewport, scissor);
	}

	VulkanPipeline::~VulkanPipeline()
	{
	}

	void VulkanPipeline::Destroy()
	{
		vkDestroyDescriptorSetLayout(m_vulkanDevice, m_descriptorLayout, nullptr);
		vkDestroyPipelineLayout(m_vulkanDevice, m_pipelineLayout, nullptr);
		vkDestroyPipeline(m_vulkanDevice, m_pipeline, nullptr);
	}

	void VulkanPipeline::CreateDescriptorSetLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders)
	{
		Insight::ShaderParser parse;
		Insight::ParsedShadeData data = parse.ParseShader(shaders[0]);

		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

		for (auto& buffer : data.UniformBlocks)
		{
			VkDescriptorSetLayoutBinding b;
			b.binding = buffer.Binding;
			b.descriptorType = buffer.GetVulkanType();
			b.descriptorCount = 1;
			b.stageFlags = data.GetVulkanShaderStage();

			descriptorSetLayoutBindings.push_back(b);
		}

		ThrowIfFailed(vkCreateDescriptorSetLayout(device, &vks::initializers::descriptorSetLayoutCreateInfo(descriptorSetLayoutBindings), nullptr, &m_descriptorLayout));
	}

	void VulkanPipeline::CreatePipelineLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders)
	{
		Insight::ShaderParser parse;
		Insight::ParsedShadeData data = parse.ParseShader(shaders[0]);

		std::vector<VkPushConstantRange> pushConstants;

		for (auto& buffer : data.PushConstants)
		{
			pushConstants.push_back(vks::initializers::pushConstantRange(data.GetVulkanShaderStage(), buffer.Size, buffer.Offset));
		}

		std::vector<VkDescriptorSetLayout> setLayouts = { m_descriptorLayout };
		ThrowIfFailed(vkCreatePipelineLayout(device, &vks::initializers::pipelineLayoutCreateInfo(setLayouts, pushConstants), nullptr, &m_pipelineLayout));
	}

	void VulkanPipeline::CreatePipeline(vks::VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor)
	{
		std::vector<std::vector<uint32_t>> compiledShaders;
		for (size_t i = 0; i < shaders.size(); ++i)
		{
			compiledShaders.push_back(CompileGLSL(shaders[i]));
		}
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(m_pipelineLayout, renderPass);

		ThrowIfFailed(vkCreateGraphicsPipelines(device, device.GetPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_pipeline));
	}

	std::vector<uint32_t> VulkanPipeline::CompileGLSL(const std::string& fileName)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		std::string glslRaw = loadShaderString(fileName);

		// Like -DMY_DEFINE=1
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult module =
			compiler.CompileGlslToSpv(glslRaw, GetShaderStage(GetSuffix(fileName)), fileName.c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << module.GetErrorMessage();
			return std::vector<uint32_t>();
		}

		return { module.cbegin(), module.cend() };
	}
}