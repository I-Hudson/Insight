#pragma once

#include "Graphics/RHI/GPUShader.h"
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

			struct Shader_Vulkan
			{
				ShaderDesc ShaderDesc;
				VertexInputLayout_Vulkan VertexInputLayout;
				std::array<vk::ShaderModule, 5> Modules;
			};

			class ShaderManger_Vulkan
			{
			public:
				ShaderManger_Vulkan();
				~ShaderManger_Vulkan();

				void SetRenderContext(RenderContext_Vulkan* context) { m_context = context; }
				Shader_Vulkan GetOrCreateShader(ShaderDesc desc);
				void Destroy();

			private:
				std::map<u64, Shader_Vulkan> m_shaders;
				RenderContext_Vulkan* m_context{ nullptr };
			};
		}
	}
}