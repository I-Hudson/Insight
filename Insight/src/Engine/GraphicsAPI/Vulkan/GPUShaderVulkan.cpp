
#include "Engine/GraphicsAPI/Vulkan/GPUShaderVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanInitializers.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanUtils.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/GraphicsAPI/Vulkan/RenderGraph/RenderGraphVulkan.h"

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
				spirv_cross::SPIRType type = spirv_cross::SPIRType();
				m_pipelineVertexInputState.VertexInputAttribute.push_back(vks::initializers::vertexInputAttributeDescription(
					0,
					inputs[i].Binding,
					(VkFormat)inputs[i].Format, inputs[i].Stride));
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
			vkDestroyDescriptorSetLayout(m_device->Device, kvp.second.Layout, nullptr);
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

			for (auto& uniformBuffer : stage.GetUniforms())
			{
				u32 bindingNumber = uniformBuffer.Binding;
				u32 setNumber = uniformBuffer.Set;
				
				VkDescriptorSetLayoutBinding setLayoutBinding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ToVulkanShaderStageFlags(stage.GetStage()), bindingNumber, 1);

				DescriptorSetLayoutData& set = m_setLayouts[setNumber];
				set.Bindings.push_back(setLayoutBinding);
				set.SetNumber = setNumber;
				set.BindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
			}

			for (auto& storageBuffer : stage.GetStorages())
			{
				u32 bindingNumber = storageBuffer.Binding;
				u32 setNumber = storageBuffer.Set;

				VkDescriptorSetLayoutBinding setLayoutBinding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ToVulkanShaderStageFlags(stage.GetStage()), bindingNumber, 1);

				DescriptorSetLayoutData& set = m_setLayouts[setNumber];
				set.Bindings.push_back(setLayoutBinding);
				set.SetNumber = setNumber;
				set.BindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
			}

			for (auto& pushConstant : stage.GetPushConstants())
			{
				ASSERT(m_pushConstant.Size == 0 && "[ParseDescriptorSetLayouts] There can only be on push constant.");
				
				m_pushConstant.Stage = stage.GetStage();
				m_pushConstant.Offset = pushConstant.Offset;
				m_pushConstant.Size += pushConstant.Size;
			}

			for (auto& sampler2D : stage.GetSamplers())
			{

				u32 bindingNumber = sampler2D.Binding;
				u32 setNumber = sampler2D.Set;

				VkDescriptorSetLayoutBinding setLayoutBinding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ToVulkanShaderStageFlags(stage.GetStage()), bindingNumber, 1);
				//if (type.array.size() == 1)
				//{
				//	setLayoutBinding.descriptorCount = (u32)type.array[0];
				//}

				DescriptorSetLayoutData& set = m_setLayouts[setNumber];
				set.Bindings.push_back(setLayoutBinding);
				set.SetNumber = setNumber;
				set.BindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
			}
		}

		for (auto& kvp : m_setLayouts)
		{
			VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = vks::initializers::descriptorSetLayoutCreateInfo(kvp.second.Bindings);
			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = { };

			if (m_device->HasExtension(Graphics::GPUDeviceExtension::Bindless_Descriptor))
			{
				//setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
				//setLayoutBindingFlags.bindingCount = kvp.second.Bindings.size();
				//std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags = {
				//	0,
				//	VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
				//};
				//setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

				bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
				bindingFlags.bindingCount = static_cast<u32>(kvp.second.Bindings.size());
				bindingFlags.pBindingFlags = kvp.second.BindingFlags.data();
				setLayoutCreateInfo.pNext = &bindingFlags;
			}
			ThrowIfFailed(vkCreateDescriptorSetLayout(m_device->Device, &setLayoutCreateInfo, nullptr, &kvp.second.Layout));
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

	void GPUPipelineVulkan::Init(Graphics::GPURenderGraphPass* graphPass, Graphics::GPUPipelineDesc& desc)
	{
		ReleaseGPU();
		m_desc = desc;

		ASSERT(m_shader && "[GPUPipelineVulkan::BuildPipeline] Shader must be valid to build a pipeline.");

		GPUShaderVulkan* shaderVulkan = static_cast<GPUShaderVulkan*>(m_shader);
		GPURenderGraphPassVulkan* graphPassVulkan = static_cast<GPURenderGraphPassVulkan*>(graphPass);

		std::vector<VkPushConstantRange> pushConstantRanges = {};
		const PushConstant& pushConstant = static_cast<GPUShaderVulkan*>(m_shader)->GetPushConstant();
		if (pushConstant.Size > 0)
		{
			pushConstantRanges.push_back(vks::initializers::pushConstantRange(ToVulkanShaderStageFlags(pushConstant.Stage), pushConstant.Size, pushConstant.Offset));
		}

		std::vector<VkDescriptorSetLayout> setLayouts;
		for (auto& set : shaderVulkan->GetDescriptorSetlayouts())
		{
			setLayouts.push_back(set.second.Layout);
		}
		ThrowIfFailed(vkCreatePipelineLayout(m_device->Device, &vks::initializers::pipelineLayoutCreateInfo(setLayouts, pushConstantRanges), nullptr, &m_layout));

		auto vertexInputInfo = shaderVulkan->GetPipelineVertexInputState();
		auto shaderStages = shaderVulkan->GetPipelineShaderStages();

		u32 outColorAttachments = 1;
		if (!graphPass->IsSwapchainPass())
		{
			outColorAttachments = (u32)graphPassVulkan->GetRenderPass().GetColorOutputs().size();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo((VkPrimitiveTopology)m_desc.PrimitiveTopologyType, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(
			(VkPolygonMode)m_desc.PolygonMode, (VkCullModeFlags)m_desc.CullMode, (VkFrontFace)m_desc.FrontFace, m_desc.DepthBaisEnabled, m_desc.DepthClampEnabled, 0);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE, outColorAttachments);
		VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(outColorAttachments, blendAttachmentStates.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(m_desc.DepthTest, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
		VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(static_cast<VkSampleCountFlagBits>(1));
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, VK_DYNAMIC_STATE_DEPTH_BIAS };
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