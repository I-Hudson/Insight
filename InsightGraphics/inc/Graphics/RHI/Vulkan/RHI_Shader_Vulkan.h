#pragma once

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
				std::vector<vk::VertexInputBindingDescription> Bindings;
				std::vector<vk::VertexInputAttributeDescription> Attributes;
				vk::PipelineVertexInputStateCreateInfo CreateInfo;
			};

			class RHI_Shader_Vulkan : public RHI_Shader
			{
			public:
				virtual ~RHI_Shader_Vulkan() override { Destroy(); }

				vk::ShaderModule GetStage(ShaderStageFlagBits stage) { return m_modules.at(stage); }
				VertexInputLayout_Vulkan GetVertexInputLayout() const { return m_vertexInputLayout; }

			private:
				virtual void Create(RenderContext* context, ShaderDesc desc) override;
				virtual void Destroy() override;

				void CompileStage(vk::ShaderStageFlagBits stageType, std::string_view path, int moduleIndex);
				void CreateVertexInputLayout(const ShaderDesc& desc);

			private:
				VertexInputLayout_Vulkan m_vertexInputLayout;
				std::array<vk::ShaderModule, 5> m_modules;
				RenderContext_Vulkan* m_context{ nullptr };

				static int s_gslangInit;
			};
		}
	}
}