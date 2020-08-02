#include "ispch.h"
#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/Vulkan.h"
#include "Insight/Renderer/Vulkan/DescriptorPool.h"
#include "Insight/Renderer/Vulkan/DescriptorSet.h"
#include "Insight/Renderer/Vulkan/Device.h"

#include "Insight/Renderer/ShaderModuleBase.h"

namespace Insight
{
	namespace Render
	{
		DescriptorPool::DescriptorPool(const Device* device, const std::vector<ParsedShadeData>& shaderData,
			const VkDescriptorPoolCreateFlags& createFlags)
			: m_device(device)
		{
			std::set<ShaderAttributeType> shaderTypes;
			for (auto it = shaderData.begin(); it != shaderData.end(); ++it)
			{
				for (auto itt = (*it).UniformBlocks.begin(); itt != (*it).UniformBlocks.end(); ++itt)
				{
					shaderTypes.insert((*itt).Type);
				}
			}

			std::vector<VkDescriptorPoolSize> pools;
			for (auto it = shaderTypes.begin(); it != shaderTypes.end(); ++it)
			{
				VkDescriptorPoolSize pool{};
				pool.type = ShaderModuleBase::GetShaderDescriptorType((*it));
				pool.descriptorCount = static_cast<uint32_t>(1);

				pools.push_back(pool);
			}

			VkDescriptorPoolCreateInfo descPoolInfo = VulkanInits::DescPoolCreateInfo(pools.size(), pools, pools.size());
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
#endif