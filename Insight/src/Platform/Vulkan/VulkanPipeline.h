#pragma once

#include "Insight/Core.h"
#include "VulkanHeader.h"

namespace Insight
{
	struct ParsedShadeData;
}

enum class PolygonMode
{
	POLYGON_MODE_FILL = 0,
	POLYGON_MODE_LINE = 1,
	POLYGON_MODE_POINT = 2,
};

namespace vks
{
	class VulkanDevice;

	class VulkanPipeline
	{
	public:
		VulkanPipeline();
		VulkanPipeline(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass);
		~VulkanPipeline();

		void Create(VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, std::vector<Insight::ParsedShadeData>& shaderData);
		void Destroy();

		void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

		VkDescriptorSetLayout& GetDescriptorLayout(int setIndex = 0) { return m_descriptorLayouts[setIndex]; }
		VkPipelineLayout& GetPipelineLayout() { return m_pipelineLayout; }
		VkPipeline& GetPipeline() { return m_pipeline; }

	private:
		void CreateDescriptorSetLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData);
		void CreatePipelineLayout(vks::VulkanDevice* device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData);
		void CreatePipeline(vks::VulkanDevice* device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, std::vector<Insight::ParsedShadeData>& shaderData);

		std::vector<uint32_t> CompileGLSL(const std::string& fileName);

	private:
		VulkanDevice* m_vulkanDevice;

		std::unordered_map<int, VkDescriptorSetLayout> m_descriptorLayouts;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		static bool s_glslInit;
	};
}