#pragma once
#ifdef IS_VULKAN
#include "Insight/Core.h"

namespace Insight
{
	namespace Render
	{
		struct UniformData;
		struct SamplerData;

		class DescriptorSet
		{
		public:
			DescriptorSet();
			~DescriptorSet();

			VkDescriptorSet* GetSet() { return &m_descSet; }
			void UpdateDescriptorSet(const VkDevice& device, const std::vector<UniformData*>& uniformDatas, 
				const std::vector<SamplerData*>& samplerDatas);

		private:
			VkDescriptorSet m_descSet;
		};
	}
}
#endif