#include "ispch.h"
#ifdef IS_VULKAN
#include "Vulkan.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Device.h"

#include "Insight/Renderer/ShaderModuleBase.h"

namespace Platform
{
	DescriptorPool::DescriptorPool(const Device* device, const std::vector<Insight::Render::ParsedShadeData>& shaderData,
		const VkDescriptorPoolCreateFlags& createFlags)
		: m_device(device)
		, m_sets(std::vector<DescriptorSet*>())
	{
		std::set<Insight::Render::ShaderAttributeType> shaderTypes;
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
			pool.type = Insight::Render::ShaderModuleBase::GetShaderDescriptorType((*it));
			pool.descriptorCount = static_cast<uint32_t>(1);

			pools.push_back(pool);
		}

		VkDescriptorPoolCreateInfo descPoolInfo = VulkanInits::DescPoolCreateInfo((const int)pools.size(), pools, pools.size());
		descPoolInfo.flags = createFlags;
		ThrowIfFailed(vkCreateDescriptorPool(m_device->GetDevice(), &descPoolInfo, nullptr, &m_pool));
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(m_device->GetDevice(), m_pool, nullptr);

		for (auto it = m_sets.begin(); it != m_sets.end(); ++it)
		{
			DELETE_ON_HEAP((*it));
		}
		m_sets.clear();
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
		vkFreeDescriptorSets(m_device->GetDevice(), m_pool, (uint32_t)sets.size(), sets.data());

		for (auto it = m_sets.begin(); it != m_sets.end(); ++it)
		{
			DELETE_ON_HEAP(*it);
		}
		m_sets.clear();
	}

	DescriptorSet* DescriptorPool::AllocDescriptorSet(const VkDescriptorSetLayout& layout)
	{
		std::vector<VkDescriptorSetLayout> layouts = { layout };
		VkDescriptorSetAllocateInfo  allocInfo = VulkanInits::DescriptorSetAllocInfo(&m_pool, 1, layouts);

		DescriptorSet* set = NEW_ON_HEAP(DescriptorSet);
		ThrowIfFailed(vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, set->GetSet()));
		m_sets.push_back(set);

		return set;
	}

	std::vector<DescriptorSet*> DescriptorPool::AllocDescriptorSets(const int& setCount, const std::vector<VkDescriptorSetLayout>& layouts)
	{
		return std::vector<DescriptorSet*>();
	}
}
#endif