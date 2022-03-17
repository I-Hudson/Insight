#pragma once

#include "Graphics/GPU/GPUShader.h"
#include "Graphics/GPU/Enums.h"
#include "GPUDevice_Vulkan.h"

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

			class GPUShader_Vulkan : public GPUShader, public GPUResource_Vulkan
			{
			public:
				GPUShader_Vulkan();
				virtual ~GPUShader_Vulkan();

				ShaderDesc GetDesc() const { return m_desc; }
				vk::ShaderModule GetStage(ShaderStageFlagBits stage) const { return m_shaderModules[stage]; }
				const VertexInputLayout_Vulkan& GetVertexInputLayout() const { return m_vertexInputLayout; }

			private:
				virtual void Create(ShaderDesc desc) override;
				virtual void Destroy() override;

				void CompileStage(vk::ShaderStageFlagBits stageType, std::string_view path, int moduleIndex);
				void CreateVertexInputLayout(const ShaderDesc& desc);

			private:
				ShaderDesc m_desc;
				VertexInputLayout_Vulkan m_vertexInputLayout;
				std::array<vk::ShaderModule, 5> m_shaderModules;

				static int s_gslangInit;
			};
		}
	}
}