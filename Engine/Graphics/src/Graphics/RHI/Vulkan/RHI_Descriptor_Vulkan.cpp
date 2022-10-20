#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Descriptor_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"
#include "Graphics/RHI/Vulkan/VulkanUtils.h"
#include "Graphics/RHI/Vulkan/RHI_Buffer_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Texture_Vulkan.h"
#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"

#include "Core/Profiler.h"

#include <vulkan/vulkan.hpp>

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
					m_context->GetDevice().destroyDescriptorSetLayout(m_layout);
					m_layout = nullptr;
				}
			}

			bool RHI_DescriptorLayout_Vulkan::ValidResouce()
			{
				return m_layout;
			}

			void RHI_DescriptorLayout_Vulkan::SetName(std::wstring name)
			{
			}

			void RHI_DescriptorLayout_Vulkan::Create(RenderContext* context, int set, DescriptorSet descriptor_sets)
			{
				m_context = static_cast<RenderContext_Vulkan*>(context);

				vk::DescriptorSetLayoutCreateInfo setCreateInfo = {};
				std::vector<vk::DescriptorSetLayoutBinding> bindingCreateInfos = {};
				std::vector<vk::DescriptorBindingFlagsEXT> bindingEXTFlags = {};

				for (const DescriptorBinding& desc : descriptor_sets.Bindings)
				{
					if (desc.Type == DescriptorType::Unknown)
					{
						continue;
					}

					vk::DescriptorSetLayoutBinding bindingInfo = {};
					bindingInfo.setBinding(desc.Binding);
					bindingInfo.setDescriptorCount(1);
					bindingInfo.setDescriptorType(DescriptorTypeToVulkan(desc.Type));
					bindingInfo.setStageFlags(ShaderStageFlagsToVulkan(desc.Stages));

					bindingCreateInfos.push_back(bindingInfo);
					if (m_context->IsExtensionEnabled(DeviceExtension::BindlessDescriptors))
					{
						bindingEXTFlags.push_back(vk::DescriptorBindingFlagBitsEXT::ePartiallyBound);
					}
				}

				vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT setEXT = {};
				if (m_context->IsExtensionEnabled(DeviceExtension::BindlessDescriptors))
				{
					setEXT.setBindingFlags(bindingEXTFlags);
					setCreateInfo.setPNext(&setEXT);
				}
				setCreateInfo.setBindings(bindingCreateInfos);
				m_layout = m_context->GetDevice().createDescriptorSetLayout(setCreateInfo);
			}
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)