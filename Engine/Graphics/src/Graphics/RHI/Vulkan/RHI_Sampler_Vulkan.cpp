#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

#include "Graphics/RHI/Vulkan/VulkanUtils.h"

#include "Core/Profiler.h"

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

				VkSamplerCreateInfo create_info = {};
				create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				create_info.magFilter = FilterToVulkan(info.MagFilter);
				create_info.minFilter = FilterToVulkan(info.MinFilter);
				create_info.mipmapMode = SamplerMipmapModeToVulkan(info.MipmapMode);
				create_info.addressModeU = SamplerAddressModeToVulkan(info.AddressMode);
				create_info.addressModeV = SamplerAddressModeToVulkan(info.AddressMode);
				create_info.addressModeW = SamplerAddressModeToVulkan(info.AddressMode);
				create_info.mipLodBias = info.MipLoadBias;
				create_info.anisotropyEnable = info.AnisotropyEnabled;
				create_info.maxAnisotropy = info.MaxAnisotropy;
				create_info.compareEnable = info.CompareEnabled;
				create_info.compareOp = CompareOpToVulkan(info.CompareOp);
				create_info.minLod = info.MinLod;
				create_info.maxLod = info.MaxLod;
				create_info.borderColor = BorderColourToVulkan(info.BoarderColour);

				VkSampler sampler = nullptr;
				ThrowIfFailed(vkCreateSampler(m_context_vulkan->GetDevice(), &create_info, nullptr, &sampler));

				RHI_Sampler* new_sampler = ::New<RHI_Sampler, Insight::Core::MemoryAllocCategory::Graphics>();
				new_sampler->Resource = sampler;
				new_sampler->Create_Info = info;

				m_samplers.emplace(hash, UPtr<RHI_Sampler>(new_sampler));
				return m_samplers.at(hash).Get();
			}

			void RHI_SamplerManager_Vulkan::ReleaseAll()
			{
				IS_PROFILE_FUNCTION();

				for (auto& pair : m_samplers)
				{
					VkSampler sampler = static_cast<VkSampler>(pair.second->Resource);
					vkDestroySampler(m_context_vulkan->GetDevice(), sampler, nullptr);
				}
				m_samplers.clear();
			}
		}
	}
}
#endif // IS_VULKAN_ENABLED