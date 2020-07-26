#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Insight/Renderer/Shader.h"

namespace Insight
{
	namespace Render
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

		class IS_API VulkanShader : public Shader
		{
		public:
			VulkanShader(ShaderData& data);
			virtual ~VulkanShader() override;

			virtual void Bind(void* context) override;
			virtual void Resize(int width, int height) override;
			virtual const ShaderData& GetData() const override { return m_shaderData; }
			
			VkDescriptorSetLayout GetDescLayout() { return m_descSetLayout; }
			VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }

		private:
			void Create(ShaderData& data);

		private:
			const Device* m_device;
			VkPipeline m_pipeline;
			ShaderData m_shaderData;
			VkPipelineLayout m_pipelineLayout;
			VkDescriptorSetLayout m_descSetLayout;
		};
	}
}
#endif