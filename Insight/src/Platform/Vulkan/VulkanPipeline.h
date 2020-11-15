#pragma once

#include "Insight/Core.h"
#include "VulkanHeader.h"

namespace vks
{
	class VulkanDevice;

	class VulkanPipeline
	{
	public:
		VulkanPipeline(VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport = VkViewport(), VkRect2D& scissor = VkRect2D());
		~VulkanPipeline();

		void Destroy();

		VkDescriptorSetLayout GetDescriptorLayout() { return m_descriptorLayout; }
		VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }
		VkPipeline GetPipeline() { return m_pipeline; }

	private:
		void CreateDescriptorSetLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders);
		void CreatePipelineLayout(vks::VulkanDevice& device, const std::vector<std::string>& shaders);
		void CreatePipeline(vks::VulkanDevice& device, const std::vector<std::string>& shaders, VkRenderPass& renderPass, VkViewport& viewport, VkRect2D& scissor);

		std::vector<uint32_t> CompileGLSL(const std::string& fileName);

	private:
		VulkanDevice& m_vulkanDevice;

		VkDescriptorSetLayout m_descriptorLayout;
		VkPipelineLayout m_pipelineLayout;
		VkPipeline m_pipeline;
	};
}