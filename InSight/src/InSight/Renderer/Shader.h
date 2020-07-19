#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Lowlevel/ShaderModuleBase.h"
#include "Insight/Renderer/Lowlevel/Renderpass.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Lowlevel/CommandBuffer.h"

namespace Insight
{
	namespace Render
	{
		class Device;

		struct ShaderData
		{
			const Device* Device;
			const std::vector<std::string> ModuleNames;
			VkExtent2D Extent;
			const Renderpass* Renderpass;
		};

		class IS_API Shader
		{
		public:
			Shader(ShaderData& data);
			~Shader();

			void Bind(CommandBuffer* commandBuffers);
			void Resize(int width, int height);

			const ShaderData GetData() { return m_shaderData; }
			const std::vector<ParsedShadeData> GetMetaData() { return m_shaderMetaData; }
			VkDescriptorSetLayout GetDescLayout() { return m_descSetLayout; }
			VkPipelineLayout GetPipelineLayout() { return m_pipelineLayout; }

		private:
			void Create(ShaderData& data);
			ShaderModuleBase& GetShaderModule(const ShaderType& type, std::vector<ShaderModuleBase>& modules);

		private:
			const Device* m_device;
			VkPipeline m_pipeline;
			ShaderData m_shaderData;
			std::vector<ParsedShadeData> m_shaderMetaData;
			VkPipelineLayout m_pipelineLayout;
			VkDescriptorSetLayout m_descSetLayout;
		};
	}
}