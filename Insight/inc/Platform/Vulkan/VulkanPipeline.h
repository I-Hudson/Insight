#pragma once

#include "Engine/Core/Core.h"
#include "VulkanHeader.h"

	struct ParsedShadeData;

namespace vks
{
	class VulkanDevice;
	struct RenderPassInfo;

	class VulkanPipeline
	{
	public:
		VulkanPipeline();
		VulkanPipeline(VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, const RenderPassInfo& renderPassInfo);
		~VulkanPipeline();

		void Create(VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, std::vector<ParsedShadeData>& shaderData, const RenderPassInfo& renderPassInfo);

		void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

		VkDescriptorSetLayout& GetDescriptorLayout(int setIndex = 0) { return m_descriptorLayouts[setIndex]; }
		VkPipelineLayout& GetPipelineLayout() { return m_pipelineLayout; }
		VkPipeline& GetPipeline() { return m_pipeline; }

	private:
		void CreateDescriptorSetLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<ParsedShadeData>& shaderData);
		void CreatePipelineLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<ParsedShadeData>& shaderData);
		void CreatePipeline(vks::VulkanDevice* device, const std::vector<std::string>& shaders, const VkRenderPass& renderPass, std::vector<ParsedShadeData>& shaderData, const RenderPassInfo& renderPassInfo);

		std::vector<uint32_t> CompileGLSL(const std::string& fileName);

	private:
		VulkanDevice* m_vulkanDevice;

		std::unordered_map<int, VkDescriptorSetLayout> m_descriptorLayouts;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
	};
}