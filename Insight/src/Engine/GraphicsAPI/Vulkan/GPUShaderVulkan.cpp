#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUShaderVulkan.h"
#include "VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"
#include <spirv_reflect.hpp>

namespace Insight::GraphicsAPI::Vulkan
{
	/// <summary>
	/// GPUShaderVulkan
	/// </summary>

	GPUShaderVulkan::GPUShaderVulkan()
		: m_pipelineShaderStages(
			std::array<VkPipelineShaderStageCreateInfo, (size_t)ShaderStage::Count>{
		VkPipelineShaderStageCreateInfo{},
			VkPipelineShaderStageCreateInfo{},
			VkPipelineShaderStageCreateInfo{},
			VkPipelineShaderStageCreateInfo{},
			VkPipelineShaderStageCreateInfo{},
			VkPipelineShaderStageCreateInfo{}})
		, m_pipelineVertexInputState(PipelineVertexInputState{})
	{
	}

	GPUShaderVulkan::~GPUShaderVulkan()
	{
		ReleaseGPU();
	}

	std::vector<VkPipelineShaderStageCreateInfo> GPUShaderVulkan::GetPipelineShaderStages()
	{
		std::vector<VkPipelineShaderStageCreateInfo> stages;
		for (auto& stage : m_pipelineShaderStages)
		{
			if (stage.module != VK_NULL_HANDLE)
			{
				stages.push_back(stage);
			}
		}
		return stages;
	}
	
	void GPUShaderVulkan::Compile()
	{
		for (auto& stage : m_stages)
		{
			stage.Parse();
		}

		Graphics::GPUShaderStage& stage = GetStage(ShaderStage::Vertex);
		auto inputs = stage.GetInputs();
		if (inputs.size() == 0)
		{
			m_pipelineVertexInputState.CreateInfo = vks::initializers::pipelineVertexInputStateCreateInfo();
		}
		else
		{
			VkVertexInputBindingDescription inputBinding;
			inputBinding.binding = 0;
			inputBinding.stride = stage.GetInputsSize();
			inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			m_pipelineVertexInputState.VertexInputBinding.push_back(inputBinding);
	
			for (size_t i = 0; i < inputs.size(); ++i)
			{
				m_pipelineVertexInputState.VertexInputAttribute.push_back(vks::initializers::vertexInputAttributeDescription(
					0,
					inputs[i].Binding,
					ToVulkanFormatFromSPRIV(inputs[i].Type, inputs[i].VecSize), inputs[i].Stride));
			}
			m_pipelineVertexInputState.CreateInfo = vks::initializers::pipelineVertexInputStateCreateInfo(m_pipelineVertexInputState.VertexInputBinding, m_pipelineVertexInputState.VertexInputAttribute);
		}
	
		CompileModules();
		ParseDescriptorSetLayouts();
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
				stage.module = nullptr;
			}
		}

		for (auto& kvp : m_setLayouts)
		{
			for (auto& set : kvp.second)
			{
				vkDestroyDescriptorSetLayout(m_device->Device, set.second.Layout, nullptr);
			}
		}
	}
	
	void GPUShaderVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (Insight::GraphicsAPI::Vulkan::GPUDebugMarkerVulkan::IsInitialised())
		{
			//Insight::GraphicsAPI::Vulkan::GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Insight::Graphics::Debug::DebugObject::Pipeline_Layout, (u64)m_pipelineLayout);
		}
	}
	
	void GPUShaderVulkan::CompileModules()
	{
		for (auto& stage : m_stages)
		{
			if (!stage.IsValid())
			{
				m_pipelineShaderStages[(size_t)stage.GetStage()] = { };
				continue;
			}
	
			// Create a shader stage with the required information.
			VkPipelineShaderStageCreateInfo shaderStage = { };
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = ToVulkanShaderStageFlags(stage.GetStage());
	
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = stage.GetRawData().size() * sizeof(uint32_t);
			moduleCreateInfo.pCode = stage.GetRawData().data();
			ThrowIfFailed(vkCreateShaderModule(m_device->Device, &moduleCreateInfo, NULL, &shaderStage.module));
			m_memoryUsage = 1;
	
			shaderStage.pName = "main";
			ASSERT(shaderStage.module != VK_NULL_HANDLE);
	
			m_pipelineShaderStages[(size_t)stage.GetStage()] = shaderStage;
		}
	}

	void GPUShaderVulkan::ParseDescriptorSetLayouts()
	{
		for (auto& stage : m_stages)
		{
			if (stage.GetRawData().empty())
			{
				continue;
			}

			std::unordered_map<u32, DescriptorSetLayoutData>& setLayouts = m_setLayouts[stage.GetStage()];
			spirv_cross::CompilerGLSL glsl(stage.GetRawData());
			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			for (auto& uniformBuffer : resources.uniform_buffers)
			{

				u32 bindingNumber = glsl.get_decoration(uniformBuffer.id, spv::Decoration::DecorationBinding);
				u32 setNumber = glsl.get_decoration(uniformBuffer.id, spv::Decoration::DecorationDescriptorSet);
				spirv_cross::SPIRType type = glsl.get_type(uniformBuffer.base_type_id);
				
				DescriptorSetLayoutData& set = setLayouts[setNumber];
				set.Bindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ToVulkanShaderStageFlags(stage.GetStage()), bindingNumber, 1));
				set.SetNumber = setNumber;
			}

			for (auto& sampler2D : resources.sampled_images)
			{

				u32 bindingNumber = glsl.get_decoration(sampler2D.id, spv::Decoration::DecorationBinding);
				u32 setNumber = glsl.get_decoration(sampler2D.id, spv::Decoration::DecorationDescriptorSet);
				spirv_cross::SPIRType type = glsl.get_type(sampler2D.base_type_id);

				DescriptorSetLayoutData& set = setLayouts[setNumber];
				set.Bindings.push_back(vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ToVulkanShaderStageFlags(stage.GetStage()), bindingNumber, 1));
				set.SetNumber = setNumber;
			}

			for (auto& kvp : setLayouts)
			{
				VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(kvp.second.Bindings);
				ThrowIfFailed(vkCreateDescriptorSetLayout(m_device->Device, &setLayoutCreateInfo, nullptr, &kvp.second.Layout));
			}
		}
	}

	/// <summary>
	/// GPUShaderVulkan
	/// </summary>
	/// 
	GPUPipelineVulkan::~GPUPipelineVulkan()
	{
		ReleaseGPU();
	}

	void GPUPipelineVulkan::SetShader(Graphics::GPUShader* shader)
	{
		m_shader = shader;
	}

	void GPUPipelineVulkan::BuildPipeline(Graphics::GPURenderGraphPass* graphPass)
	{
		ReleaseGPU();

		ASSERT(m_shader && "[GPUPipelineVulkan::BuildPipeline] Shader must be valid to build a pipeline.");

		GPUShaderVulkan* shaderVulkan = static_cast<GPUShaderVulkan*>(m_shader);
		GPURenderGraphPassVulkan* graphPassVulkan = static_cast<GPURenderGraphPassVulkan*>(graphPass);

		std::vector<VkPushConstantRange> pushConstants;
		/*for (auto& data : )
		{
			for (auto& buffer : data.PushConstants)
			{
				pushConstants.push_back(vks::initializers::pushConstantRange(data.GetVulkanShaderStage(), buffer.Size, buffer.Offset));
			}
		}*/

		std::vector<VkDescriptorSetLayout> setLayouts;
		for (auto& stage : shaderVulkan->GetDescriptorSetlayouts())
		{
			for (auto& set : stage.second)
			{
				setLayouts.push_back(set.second.Layout);
			}
		}
		ThrowIfFailed(vkCreatePipelineLayout(m_device->Device, &vks::initializers::pipelineLayoutCreateInfo(setLayouts, pushConstants), nullptr, &m_layout));

		auto vertexInputInfo = shaderVulkan->GetPipelineVertexInputState();
		auto shaderStages = shaderVulkan->GetPipelineShaderStages();

		u32 outColorAttachments = 1;
		if (!graphPass->IsSwapchainPass())
		{
			outColorAttachments = graphPassVulkan->GetRenderPass().GetColorOutputs().size();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);;
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE, outColorAttachments);
		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(outColorAttachments, blendAttachmentStates.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(static_cast<VkSampleCountFlagBits>(1));
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

		// TODO: remove this and place into a Desc struct.
		if (vertexInputInfo.vertexBindingDescriptionCount == 0)
		{
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(m_layout, graphPassVulkan->GetRenderPassVulkan());
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;

		ThrowIfFailed(vkCreateGraphicsPipelines(m_device->Device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_pipeline));
		m_memoryUsage = 8;
	}

	void GPUPipelineVulkan::SetName(const std::string& name)
	{
		m_name = name;
		if (GPUDebugMarkerVulkan::Instance()->IsInitialised())
		{
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Pipeline, (u64)m_pipeline);
			GPUDebugMarkerVulkan::Instance()->SetObjectName(m_name, Graphics::Debug::DebugObject::Pipeline_Layout, (u64)m_layout);
		}
	}

	void GPUPipelineVulkan::OnReleaseGPU()
	{
		if (m_pipeline)
		{
			vkDestroyPipeline(m_device->Device, m_pipeline, nullptr);
		}
		if (m_layout)
		{
			vkDestroyPipelineLayout(m_device->Device, m_layout, nullptr);
		}
	}
}