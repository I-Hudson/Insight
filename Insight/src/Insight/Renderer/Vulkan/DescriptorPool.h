#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

namespace Insight
{
	namespace Render
	{
		class Device;
		class DescriptorSet;
		struct ParsedShadeData;

		class DescriptorPool
		{
		public:
			DescriptorPool(const Device* device, const std::vector<ParsedShadeData>& shaderData,
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
}
#endif