#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			void RHI_SamplerManager_Vulkan::SetRenderContext(RenderContext* context)
			{
				m_context_vulkan = static_cast<RenderContext_Vulkan*>(context);
			}

			RHI_Sampler* RHI_SamplerManager_Vulkan::GetOrCreateSampler(RHI_SamplerCreateInfo info)
			{
				const u64 hash = info.GetHash();
				if (auto itr = m_samplers.find(hash); itr != m_samplers.end())
				{
					return itr->second.Get();
				}

				vk::SamplerCreateInfo create_info = vk::SamplerCreateInfo(
					{},
					FilterToVulkan(info.MagFilter),
					FilterToVulkan(info.MinFilter),
					SamplerMipmapModeToVulkan(info.MipmapMode),
					SamplerAddressModeToVulkan(info.AddressMode),
					SamplerAddressModeToVulkan(info.AddressMode),
					SamplerAddressModeToVulkan(info.AddressMode),
					info.MipLoadBias,
					info.AnisotropyEnabled,
					info.MaxAnisotropy,
					info.CompareEnabled,
					CompareOpToVulkan(info.CompareOp),
					info.MinLod,
					info.MaxLod,
					BorderColourToVulkan(info.BoarderColour));

				vk::Sampler sampler = m_context_vulkan->GetDevice().createSampler(create_info);

				RHI_Sampler* new_sampler = NewTracked(RHI_Sampler);
				new_sampler->Resource = *reinterpret_cast<VkSampler*>(&sampler);
				new_sampler->Create_Info = info;

				m_samplers.emplace(hash, UPtr<RHI_Sampler>(new_sampler));
				return m_samplers.at(hash).Get();
			}

			void RHI_SamplerManager_Vulkan::ReleaseAll()
			{
				for (auto& pair : m_samplers)
				{
					vk::Sampler sampler = *reinterpret_cast<vk::Sampler*>(&pair.second->Resource);
					m_context_vulkan->GetDevice().destroySampler(sampler);
				}
				m_samplers.clear();
			}
		}
	}
}