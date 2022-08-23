#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"
#include "Graphics/RHI/Vulkan/RenderContext_Vulkan.h"

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

			RHI_Sampler RHI_SamplerManager_Vulkan::GetOrCreateSampler(RHI_SamplerCreateInfo info)
			{
				const u64 hash = info.GetHash();
				if (auto itr = m_samplers.find(hash); itr != m_samplers.end())
				{
					return itr->second;
				}

				vk::SamplerCreateInfo create_info = vk::SamplerCreateInfo(
					{},
					vk::Filter::eLinear,
					vk::Filter::eLinear,
					vk::SamplerMipmapMode::eLinear,
					vk::SamplerAddressMode::eMirroredRepeat,
					vk::SamplerAddressMode::eMirroredRepeat,
					vk::SamplerAddressMode::eMirroredRepeat,
					0.0f,
					false,
					1.0f,
					false,
					vk::CompareOp::eNever,
					0.0f,
					0.0f,
					vk::BorderColor::eFloatOpaqueWhite);

				vk::Sampler sampler = m_context_vulkan->GetDevice().createSampler(create_info);
				m_samplers[hash] = *reinterpret_cast<RHI_Sampler*>(&sampler);
				return m_samplers.at(hash);
			}

			void RHI_SamplerManager_Vulkan::ReleaseAll()
			{
				for (auto& pair : m_samplers)
				{
					vk::Sampler sampler = *reinterpret_cast<vk::Sampler*>(&pair.second);
					m_context_vulkan->GetDevice().destroySampler(sampler);
				}
				m_samplers.clear();
			}
		}
	}
}