#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_PipelineLayout_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			RHI_PipelineLayout_Vulkan::~RHI_PipelineLayout_Vulkan()
			{
				Release();
			}

			VkPipelineLayout RHI_PipelineLayout_Vulkan::GetPipelineLayout() const
			{
				return m_pipelineLayout;
			}

			void RHI_PipelineLayout_Vulkan::Create(RenderContext* context, PipelineStateObject pso)
			{
				CreateLayout(context, pso.Shader, pso.Name);
			}

			void RHI_PipelineLayout_Vulkan::Create(RenderContext* context, ComputePipelineStateObject pso)
			{
				CreateLayout(context, pso.Shader, pso.Name);
			}

			void RHI_PipelineLayout_Vulkan::Release()
			{
				if (m_pipelineLayout != VK_NULL_HANDLE)
				{
					vkDestroyPipelineLayout(m_context->GetDevice(), m_pipelineLayout, nullptr);
					m_pipelineLayout = VK_NULL_HANDLE;
				}
			}

			bool RHI_PipelineLayout_Vulkan::ValidResource()
			{
				return m_pipelineLayout != VK_NULL_HANDLE;
			}

			void RHI_PipelineLayout_Vulkan::SetName(std::string name)
			{
				if (m_pipelineLayout)
				{
					m_context->SetObjectName(name, (u64)m_pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT);
				}
			}

			void RHI_PipelineLayout_Vulkan::CreateLayout(RenderContext* context, RHI_Shader* shader, std::string name)
			{
				m_context = static_cast<RenderContext_Vulkan*>(context);

				const std::vector<DescriptorSet> descriptor_sets = shader->GetDescriptorSets();
				std::vector<VkDescriptorSetLayout> set_layouts = {};
				std::vector<DescriptorSet> current_descriptor_sets;

				for (const DescriptorSet& descriptor_set : descriptor_sets)
				{
					RHI_DescriptorLayout_Vulkan* layoutVulkan = static_cast<RHI_DescriptorLayout_Vulkan*>(m_context->GetDescriptorLayoutManager().GetLayout(descriptor_set));
					set_layouts.push_back(layoutVulkan->GetLayout());
				}

				PushConstant push_constant = shader->GetPushConstant();
				std::vector<VkPushConstantRange> push_constants;
				if (push_constant.Size > 0)
				{
					VkPushConstantRange pushConstantRange = {};
					pushConstantRange.stageFlags = ShaderStageFlagsToVulkan(push_constant.ShaderStages);
					pushConstantRange.offset = push_constant.Offset;
					pushConstantRange.size = push_constant.Size;
					push_constants.push_back(pushConstantRange);
				}

				VkPipelineLayoutCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				createInfo.pSetLayouts = set_layouts.data();
				createInfo.setLayoutCount = static_cast<u32>(set_layouts.size());
				createInfo.pPushConstantRanges = push_constants.data();
				createInfo.pushConstantRangeCount = static_cast<u32>(push_constants.size());

				ThrowIfFailed(vkCreatePipelineLayout(m_context->GetDevice(), &createInfo, nullptr, &m_pipelineLayout));
				SetName(name + "_Layout");
			}
		}
	}
}

#endif // IS_VULKAN_ENABLED