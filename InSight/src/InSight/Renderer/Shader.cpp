#include "ispch.h"
#include "Shader.h"

#include "Insight/Renderer/Lowlevel/Device.h"

#include "VulkanInits.h"

namespace Insight
{
	namespace Render
	{
		Shader::Shader(ShaderData& data)
			: m_device(data.Device)
		{
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDescriptorSetLayoutBinding > shaderDescriptorLayouts;

			for (auto it = data.Modules.begin(); it != data.Modules.end(); ++it)
			{
				VkPipelineShaderStageCreateInfo info = VulkanInits::ShaderPipelineInfo();
				info.stage = (*it).GetShaderStageBit();
				info.module = (*it).GetModule();

				auto uniforms = (*it).GetPipelineLayoutCreateInfo();
				for (auto it = uniforms.begin(); it != uniforms.end(); ++it)
				{
					shaderDescriptorLayouts.push_back(*it);
				}
				shaderStages.push_back(info);
			}

			VkPipelineVertexInputStateCreateInfo vertexCreateInfo = data.Modules[0].GetVertexInputCreateInfo();
			VkPipelineInputAssemblyStateCreateInfo inputAssembly = VulkanInits::PipelineInputAssemblyInfo();

			VkViewport viewPort
			{ 0.0f, 0.0f, static_cast<float>(data.Extent.width), static_cast<float>(data.Extent.height), 0.0f, 1.0f };
			VkRect2D scissor{ {0, 0}, data.Extent };
			VkPipelineViewportStateCreateInfo viewportInfo = VulkanInits::PipelineViewportInfo(viewPort, scissor);
			VkPipelineRasterizationStateCreateInfo rasterizationInfo = VulkanInits::PipelineRasterizationInfo();
			VkPipelineMultisampleStateCreateInfo multisampleInfo = VulkanInits::PipelineMutisampleInfo();
			VkPipelineColorBlendAttachmentState colourBlendAttachState = VulkanInits::PipelineColourBlendState();
			std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachStates;
			for (int i = 0; i < GetShaderModule(ShaderType::Fragment, data.Modules).GetData().OutAttri.Attributes.size(); ++i)
			{
				colourBlendAttachStates.push_back(VulkanInits::PipelineColourBlendState());
			}
			VkPipelineColorBlendStateCreateInfo colourBlendAttachInfo = VulkanInits::PipelineColourBlendInfo(colourBlendAttachStates);
			VkPipelineDynamicStateCreateInfo dynamicInfo = VulkanInits::PipelineDynamicState({ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH });
			VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = VulkanInits::DescriptorSetLayoutCreateInfo(shaderDescriptorLayouts);
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanInits::PipelineLayoutInfo({ });
			ThrowIfFailed(vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

			VkGraphicsPipelineCreateInfo graphicsInfo = VulkanInits::GraphicsPipelineInfo(&shaderStages, &vertexCreateInfo, &inputAssembly, &viewportInfo,
																						  &rasterizationInfo, &multisampleInfo, nullptr, &colourBlendAttachInfo,
																						  nullptr, m_pipelineLayout, 
																						  data.Renderpass->GetRenderpass(), 0);

			ThrowIfFailed(vkCreateGraphicsPipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &graphicsInfo, nullptr, &m_pipeline));

			for (auto it = data.Modules.begin(); it != data.Modules.end(); ++it)
			{
				(*it).Destroy();
			}
		}

		Shader::~Shader()
		{
			vkDestroyPipeline(m_device->GetDevice(), m_pipeline, nullptr);
			vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);
		}

		void Shader::Bind(CommandBuffer* commandBuffers)
		{
			vkCmdBindPipeline(commandBuffers->GetBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		}

		ShaderModuleBase& Shader::GetShaderModule(const ShaderType& type, std::vector<ShaderModuleBase>& modules)
		{
			for (auto it = modules.begin(); it != modules.end(); ++it)
			{
				if ((*it).GetShaderType() == type)
				{
					return *it;
				}
			}
		}
	}
}