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
		VulkanPipeline(VulkanDevice& device);
		VulkanPipeline(VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor);
		~VulkanPipeline();

		void Create(VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor);
		void Destroy();

		void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

		VkDescriptorSetLayout GetDescriptorLayout() { return m_descriptorLayout; }
		VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }
		VkPipeline GetPipeline() { return m_pipeline; }

	private:
		void CreateDescriptorSetLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData);
		void CreatePipelineLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders, std::vector<Insight::ParsedShadeData>& shaderData);
		void CreatePipeline(vks::VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor, std::vector<Insight::ParsedShadeData>& shaderData);

		std::vector<uint32_t> CompileGLSL(const std::string& fileName);

	private:
		VulkanDevice& m_vulkanDevice;

		VkDescriptorSet m_descriptorSet;
		VkDescriptorSetLayout m_descriptorLayout;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;

		static bool s_glslInit;
	};
}