#pragma once

#include "Insight/Core.h"
#include "Insight/Renderer/Vulkan.h"

namespace Insight
{
	namespace Render
	{
		struct UniformData;

		class DescriptorSet
		{
		public:
			DescriptorSet();
			~DescriptorSet();

			VkDescriptorSet* GetSet() { return &m_descSet; }
			void UpdateDescriptorSet(const VkDevice& device, const std::vector<UniformData*>& uniformDatas);

		private:
			VkDescriptorSet m_descSet;
		};
	}
}
