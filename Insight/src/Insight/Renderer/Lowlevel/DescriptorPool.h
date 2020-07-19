#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Vulkan.h"
#include "Insight/Renderer/Lowlevel/DescriptorSet.h"

#include <vector>

namespace Insight
{
	namespace Render
	{
		class Device;

		class DescriptorPool
		{
		public:
			DescriptorPool(const Device* device, const int& descriptorCount, const int& maxDescriptorSetCount,
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