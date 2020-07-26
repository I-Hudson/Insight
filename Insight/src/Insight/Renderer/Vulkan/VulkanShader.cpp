#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/VulkanShader.h"
#include "Insight/Renderer/Vulkan/Device.h"
#include "Insight/Renderer/ShaderModuleBase.h"
#include "Insight/Renderer/Vulkan/Renderpass.h"
#include "Insight/Renderer/Vulkan/CommandBuffer.h"

namespace Insight
{
	namespace Render
	{
		VulkanShader::VulkanShader(ShaderData& data)
			: m_device(data.Device) 
			, m_shaderData(data)
		{
			Create(data);
		}

		VulkanShader::~VulkanShader()
		{
			vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_descSetLayout, nullptr);
			vkDestroyPipeline(m_device->GetDevice(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);
		}

		void VulkanShader::Bind(void* context)
		{
			vkCmdBindPipeline(static_cast<CommandBuffer*>(context)->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		}

		void VulkanShader::Resize(int width, int height)
		{
			vkDestroyPipeline(m_device->GetDevice(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);

			m_shaderData.Extent = VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			Create(m_shaderData);
		}

		void VulkanShader::Create(ShaderData& data)
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDescriptorSetLayoutBinding > shaderDescriptorLayouts;
			std::vector<VkPushConstantRange> pushContants;

			std::vector<ShaderModuleBase> modules;
			for (auto it = data.ModuleNames.begin(); it != data.ModuleNames.end(); ++it)
			{
				modules.push_back(ShaderModuleBase(m_device, (*it)));
				m_shaderMetaData.push_back(modules[modules.size() - 1].GetData());
			}

			for (auto it = modules.begin(); it != modules.end(); ++it)
			{
				VkPipelineShaderStageCreateInfo info = VulkanInits::ShaderPipelineInfo();
				info.stage = (*it).GetShaderStageBit();
				info.module = (*it).GetModule();

				auto uniforms = (*it).GetPipelineLayoutCreateInfo();
				for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
				{
					shaderDescriptorLayouts.push_back(*it);
				}

				auto pushContantsFromModule = (*it).GetPushContants();
				for (auto it = pushContantsFromModule.begin(); it != pushContantsFromModule.end(); ++it)
				{
					pushContants.push_back(*it);
				}

				shaderStages.push_back(info);
			}

			auto vertexInputBindingsDescs = GetShaderModule(ShaderType::VertexShader, modules).GetVertexBindingDesc();
			auto vertexInputAttribBindings = GetShaderModule(ShaderType::VertexShader, modules).GetAttributes();
			VkPipelineVertexInputStateCreateInfo vertexCreateInfo = VulkanInits::VertexInputInfo(vertexInputBindingsDescs, vertexInputAttribBindings);
			VkPipelineInputAssemblyStateCreateInfo inputAssembly = VulkanInits::PipelineInputAssemblyInfo();

			VkViewport viewPort
			{ 0.0f, 0.0f, static_cast<float>(data.Extent.width), static_cast<float>(data.Extent.height), 0.0f, 1.0f };
			VkRect2D scissor{ {0, 0}, data.Extent };
			VkPipelineViewportStateCreateInfo viewportInfo = VulkanInits::PipelineViewportInfo(&viewPort, &scissor);

			VkPipelineRasterizationStateCreateInfo rasterizationInfo = VulkanInits::PipelineRasterizationInfo();
			VkPipelineMultisampleStateCreateInfo multisampleInfo = VulkanInits::PipelineMutisampleInfo();
			VkPipelineColorBlendAttachmentState colourBlendAttachState = VulkanInits::PipelineColourBlendState();

			std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachStates;
			for (int i = 0; i < GetShaderModule(ShaderType::FragmentShader, modules).GetData().OutAttri.Attributes.size(); ++i)
			{
				colourBlendAttachStates.push_back(VulkanInits::PipelineColourBlendState());
			}
			VkPipelineColorBlendStateCreateInfo colourBlendAttachInfo = VulkanInits::PipelineColourBlendInfo(colourBlendAttachStates);

			VkPipelineDepthStencilStateCreateInfo depthSteniclCreateInfo = VulkanInits::PipelineDepthStencilInfo();

			std::vector<VkDynamicState> dynamicStats = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicInfo = VulkanInits::PipelineDynamicState(dynamicStats);

			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = VulkanInits::DescriptorSetLayoutCreateInfo(shaderDescriptorLayouts);
			ThrowIfFailed(vkCreateDescriptorSetLayout(m_device->GetDevice(), &descriptorLayoutInfo, nullptr, &m_descSetLayout));

			std::vector<VkDescriptorSetLayout> layouts = { m_descSetLayout };
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanInits::PipelineLayoutInfo(layouts, pushContants);
			ThrowIfFailed(vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

			VkGraphicsPipelineCreateInfo graphicsInfo = VulkanInits::GraphicsPipelineInfo(&shaderStages, &vertexCreateInfo, &inputAssembly, &viewportInfo,
				&rasterizationInfo, &multisampleInfo, &depthSteniclCreateInfo, &colourBlendAttachInfo,
				nullptr, m_pipelineLayout,
				data.Renderpass->GetRenderpass(), 0);

			ThrowIfFailed(vkCreateGraphicsPipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &graphicsInfo, nullptr, &m_pipeline));

			for (auto it = modules.begin(); it != modules.end(); ++it)
			{
				(*it).Destroy();
			}
		}
	}
}
#endif