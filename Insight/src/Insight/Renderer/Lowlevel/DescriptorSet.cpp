#include "ispch.h"
#include "DescriptorSet.h"

#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Renderer/VulkanMaterial.h"

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

		void DescriptorSet::UpdateDescriptorSet(const VkDevice& device, const std::vector<UniformData*>& uniformDatas)
		{
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			std::vector<VkDescriptorBufferInfo> bufferInfos;

			int offset = 0;
			for (auto it = uniformDatas.begin(); it != uniformDatas.end(); ++it)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = (*it)->Buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = (*it)->Size;

				bufferInfos.push_back(bufferInfo);
			}

			for (UINT i = 0; i < uniformDatas.size(); ++i)
			{
				VkWriteDescriptorSet descriptorWrite = VulkanInits::WriteDescriptorSet(m_descSet, uniformDatas[i]->Binding, 0,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfos[i]);
				writeDescriptorSets.push_back(descriptorWrite);
			}

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}
}