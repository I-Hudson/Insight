#include "ispch.h"
#include "DescriptorPool.h"

#include "Insight/Renderer/VulkanInits.h"
#include "Insight/Renderer/Lowlevel/Device.h"
#include "Insight/Memory/MemoryManager.h"

namespace Insight
{
	namespace Render
	{
		DescriptorPool::DescriptorPool(const Device* device, const int& descriptorCount, const int& maxDescriptorSetCount,
			const VkDescriptorPoolCreateFlags& createFlags)
			: m_device(device)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = static_cast<uint32_t>(descriptorCount);

			VkDescriptorPoolCreateInfo descPoolInfo = VulkanInits::DescPoolCreateInfo(1, poolSize, maxDescriptorSetCount);
			descPoolInfo.flags = createFlags;
			ThrowIfFailed(vkCreateDescriptorPool(m_device->GetDevice(), &descPoolInfo, nullptr, &m_pool));
		}

		DescriptorPool::~DescriptorPool()
		{
			vkDestroyDescriptorPool(m_device->GetDevice(), m_pool, nullptr);

			for (auto it = m_sets.begin(); it != m_sets.end(); ++it)
			{
				Memory::MemoryManager::DeleteOnFreeList((*it));
			}
		}

		void DescriptorPool::FreeDescriptorSets()
		{
			if (m_sets.size() == 0)
			{
				return;
			}

			std::vector<VkDescriptorSet> sets;
			for (auto it = m_sets.begin(); it != m_sets.end(); ++it)
			{
				sets.push_back(*(*it)->GetSet());
			}
			vkFreeDescriptorSets(m_device->GetDevice(), m_pool, sets.size(), sets.data());

			for (auto it = m_sets.begin(); it != m_sets.end(); ++it)
			{
				Memory::MemoryManager::DeleteOnFreeList(*it);
			}
			m_sets.clear();
		}

		DescriptorSet* DescriptorPool::AllocDescriptorSet(const VkDescriptorSetLayout& layout)
		{
			std::vector<VkDescriptorSetLayout> layouts = {layout};
			VkDescriptorSetAllocateInfo  allocInfo = VulkanInits::DescriptorSetAllocInfo(&m_pool, 1, layouts);
			
			DescriptorSet* set = Memory::MemoryManager::NewOnFreeList<DescriptorSet>();
			ThrowIfFailed(vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, set->GetSet()));
			m_sets.push_back(set);

			return set;
		}

		std::vector<DescriptorSet*> DescriptorPool::AllocDescriptorSets(const int& setCount, const std::vector<VkDescriptorSetLayout>& layouts)
		{
			return std::vector<DescriptorSet*>();
		}
	}
}