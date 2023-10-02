#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include <map>
#include <array>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			struct VertexInputLayout_Vulkan
			{
				std::vector<VkVertexInputBindingDescription> Bindings;
				std::vector<VkVertexInputAttributeDescription> Attributes;
				VkPipelineVertexInputStateCreateInfo CreateInfo;
			};

			class RHI_Shader_Vulkan : public RHI_Shader
			{
			public:
				virtual ~RHI_Shader_Vulkan() override { Destroy(); }

				VkShaderModule GetStage(ShaderStageFlagBits stage) const;
				const VertexInputLayout_Vulkan& GetVertexInputLayout() const { return m_vertexInputLayout; }
				std::string_view GetMainFuncName(ShaderStageFlagBits stage) const;

			private:
				virtual void Create(RenderContext* context, ShaderDesc desc) override;
				virtual void Destroy() override;

				void CompileStage(ShaderStageFlagBits stage, std::string_view path, int moduleIndex);
				void CompileStage(ShaderStageFlagBits stage, std::string_view name, const std::vector<Byte>& shaderData, int moduleIndex);

				void CreateVertexInputLayout(const ShaderDesc& desc);
				void CreateShaderModule(IDxcBlob* code, int moduleIndex, ShaderCompiler& compiler, ShaderStageFlagBits stage);

			private:
				VertexInputLayout_Vulkan m_vertexInputLayout;
				std::array<VkShaderModule, 5> m_modules;
				std::array<std::string, 5> m_mainFuncNames;
				RenderContext_Vulkan* m_context{ nullptr };
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)