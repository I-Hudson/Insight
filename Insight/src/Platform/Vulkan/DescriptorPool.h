#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"
#include "Vulkan.h"

namespace Insight
{
	namespace Render
	{
		struct ParsedShadeData;
	}
}

namespace Platform
{
	class Device;
	class DescriptorSet;

	class DescriptorPool
	{
	public:
		DescriptorPool(const Device* device, const std::vector<Insight::Render::ParsedShadeData>& shaderData,
			const VkDescriptorPoolCreateFlags& createFlags = 0);
		~DescriptorPool();

		VkDescriptorPool& GetDescriptorPool() { return m_pool; }
		void FreeDescriptorSets();

		DescriptorSet* AllocDescriptorSet(const VkDescriptorSetLayout& layout);
		std::vector<DescriptorSet*> AllocDescriptorSets(const int& setCount, const std::vector<VkDescriptorSetLayout>& layouts);

	private:
		const Device* m_device;
		VkDescriptorPool m_pool;

		std::vector<DescriptorSet*> m_sets;
	};
}
#endif