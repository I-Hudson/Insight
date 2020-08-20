#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/DescriptorSet.h"
#include "Insight/Renderer/Vulkan/VulkanMaterial.h"

namespace Insight
{
	namespace Render
	{
		DescriptorSet::DescriptorSet()
		{
		}
		DescriptorSet::~DescriptorSet()
		{
		}

		void DescriptorSet::UpdateDescriptorSet(const VkDevice& device, const std::vector<UniformData*>& uniformDatas, 
			const std::vector<SamplerData*>& samplerDatas)
		{
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			std::vector<VkDescriptorBufferInfo> bufferInfos;
			std::vector<VkDescriptorImageInfo> imageInfos;

			int offset = 0;
			for (auto it = uniformDatas.begin(); it != uniformDatas.end(); ++it)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = (*it)->Buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = (*it)->Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? (*it)->Size : VK_WHOLE_SIZE;
				bufferInfos.push_back(bufferInfo);
			}

			for (auto it = samplerDatas.begin(); it != samplerDatas.end(); ++it)
			{
				VkDescriptorImageInfo imageInfo;
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = *(*it)->ImageView;
				imageInfo.sampler = *(*it)->Sampler;

				imageInfos.push_back(imageInfo);
			}

			for (UINT i = 0; i < uniformDatas.size(); ++i)
			{
				VkWriteDescriptorSet descriptorWrite = VulkanInits::WriteDescriptorSet(m_descSet, uniformDatas[i]->Binding, 0,
					uniformDatas[i]->Type, 1, &bufferInfos[i], nullptr);
				writeDescriptorSets.push_back(descriptorWrite);
			}
			for (UINT i = 0; i < samplerDatas.size(); ++i)
			{
				VkWriteDescriptorSet descriptorWrite = VulkanInits::WriteDescriptorSet(m_descSet, samplerDatas[i]->Binding, 0,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, nullptr, &imageInfos[i]);
				writeDescriptorSets.push_back(descriptorWrite);
			}

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}
}
#endif