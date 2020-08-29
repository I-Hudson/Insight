#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"
#include "glm/glm.hpp"

namespace Platform
{
	class Device;
	class Renderpass;

	struct ShaderData
	{
		const Device* Device;
		const std::vector<std::string> ModuleNames;
		VkExtent2D Extent;
		const Renderpass* Renderpass;
	};

	class IS_API VulkanShader : public Insight::Render::Shader
	{
	public:
		VulkanShader(const Device* device, const std::vector<std::string>& shaderPaths, glm::ivec2 extent, const Renderpass* renderpass);
		virtual ~VulkanShader() override;

		virtual void Bind(void* context) override;
		virtual void Resize(int width, int height) override;

		VkDescriptorSetLayout GetDescLayout() { return m_descSetLayout; }
		VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }

	private:
		void Create();

	private:
		const Device* m_device;
		std::vector<std::string> m_shaders;
		VkExtent2D m_extent;
		const Renderpass* m_renderpass;

		VkPipeline m_pipeline;
		VkPipelineLayout m_pipelineLayout;
		VkDescriptorSetLayout m_descSetLayout;
	};
}
#endif