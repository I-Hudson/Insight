#include "ispch.h"
#include "GPUShaderVulkan.h"
#include "VulkanInitializers.h"
#include "VulkanUtils.h"
#include "GPUDeviceVulkan.h"

GPUShaderVulkan::GPUShaderVulkan()
	: m_pipelineShaderStages(
		std::array<VkPipelineShaderStageCreateInfo, (size_t)ShaderStage::Count>{
		VkPipelineShaderStageCreateInfo{},
		VkPipelineShaderStageCreateInfo{},
		VkPipelineShaderStageCreateInfo{},
		VkPipelineShaderStageCreateInfo{},
		VkPipelineShaderStageCreateInfo{},
		VkPipelineShaderStageCreateInfo{}})
	, m_pipelineVertexInputState(VkPipelineVertexInputStateCreateInfo{})
{
}

GPUShaderVulkan::~GPUShaderVulkan()
{
}

void GPUShaderVulkan::Compile()
{
	GPUShaderStage& stage = GetStage(ShaderStage::Vertex);
	if (!stage.IsValid())
	{
		stage.Parse();
	}
	auto inputs = stage.GetInputs();
	if (inputs.size() == 0)
	{
		m_pipelineVertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
	}
	else
	{
		std::vector<VkVertexInputBindingDescription> VertexInputBinding;
		std::vector<VkVertexInputAttributeDescription> VertexInputAttribute;

		VkVertexInputBindingDescription inputBinding;
		inputBinding.binding = 0;
		inputBinding.stride = stage.GetInputsSize();
		inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		VertexInputBinding.push_back(inputBinding);

		for (size_t i = 0; i < inputs.size(); ++i)
		{
			VertexInputAttribute.push_back(vks::initializers::vertexInputAttributeDescription(
				0,
				inputs[i].Binding,
				ToVulkanFormatFromSPRIV(inputs[i].Type, inputs[i].VecSize), inputs[i].Stride));
		}
		m_pipelineVertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo(VertexInputBinding, VertexInputAttribute);
	}

	CompileModules();
}

void GPUShaderVulkan::ReleaseGPUResoucesEarly()
{
	OnReleaseGPU();
}

void GPUShaderVulkan::OnReleaseGPU()
{
	for (auto& stage : m_pipelineShaderStages)
	{
		if (stage.module != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(m_device->Device, stage.module, nullptr);
			stage.module = VK_NULL_HANDLE;
		}
	}
}

void GPUShaderVulkan::CompileModules()
{
	for (auto& stage : m_stages)
	{
		if (!stage.IsValid())
		{
			continue;
		}

		// Create a shader stage with the required information.
		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = ToVulkanShaderStageFlags(stage.GetStage());

		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = stage.GetRawData().size() * sizeof(uint32_t);
		moduleCreateInfo.pCode = stage.GetRawData().data();
		ThrowIfFailed(vkCreateShaderModule(m_device->Device, &moduleCreateInfo, NULL, &shaderStage.module));
	
		shaderStage.pName = "main";
		ASSERT(shaderStage.module != VK_NULL_HANDLE);

		m_pipelineShaderStages[(size_t)stage.GetStage()] = shaderStage;
	}
}
