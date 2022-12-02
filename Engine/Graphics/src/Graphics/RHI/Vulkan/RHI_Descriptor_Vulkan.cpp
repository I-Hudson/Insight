#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"

#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			//// <summary>
			//// RHI_DescriptorLayout_Vulkan
			//// </summary>
			void RHI_DescriptorLayout_Vulkan::Release()
			{
				if (m_layout)
				{
					vkDestroyDescriptorSetLayout(m_context->GetDevice(), m_layout, nullptr);
					m_layout = nullptr;
				}
			}

			bool RHI_DescriptorLayout_Vulkan::ValidResouce()
			{
				return m_layout;
			}

			void RHI_DescriptorLayout_Vulkan::SetName(std::string name)
			{
			}

			void RHI_DescriptorLayout_Vulkan::Create(RenderContext* context, int set, DescriptorSet descriptor_sets)
			{
				m_context = static_cast<RenderContext_Vulkan*>(context);

				VkDescriptorSetLayoutCreateInfo setCreateInfo = {};
				setCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				std::vector<VkDescriptorSetLayoutBinding> bindingCreateInfos = {};
				std::vector<VkDescriptorBindingFlagsEXT> bindingEXTFlags = {};

				for (const DescriptorBinding& desc : descriptor_sets.Bindings)
				{
					if (desc.Type == DescriptorType::Unknown)
					{
						continue;
					}

					VkDescriptorSetLayoutBinding bindingInfo = {};
					bindingInfo.binding = desc.Binding;
					bindingInfo.descriptorCount = 1;
					bindingInfo.descriptorType = DescriptorTypeToVulkan(desc.Type);
					bindingInfo.stageFlags = ShaderStageFlagsToVulkan(desc.Stages);

					bindingCreateInfos.push_back(bindingInfo);
					if (m_context->IsExtensionEnabled(DeviceExtension::BindlessDescriptors))
					{
						bindingEXTFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
					}
				}

				VkDescriptorSetLayoutBindingFlagsCreateInfoEXT setEXT = {};
				setEXT.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
				if (m_context->IsExtensionEnabled(DeviceExtension::BindlessDescriptors))
				{
					setEXT.pBindingFlags = bindingEXTFlags.data();
					setEXT.bindingCount = static_cast<u32>(bindingEXTFlags.size());
					setCreateInfo.pNext = &setEXT;
				}
				setCreateInfo.pBindings = bindingCreateInfos.data();
				setCreateInfo.bindingCount = static_cast<u32>(bindingCreateInfos.size());
				ThrowIfFailed(vkCreateDescriptorSetLayout(m_context->GetDevice(), &setCreateInfo, nullptr, &m_layout));
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)