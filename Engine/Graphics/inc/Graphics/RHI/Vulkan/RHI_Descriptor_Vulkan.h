#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/PipelineStateObject.h"

#include <vulkan/vulkan.h>

#include <unordered_set>
#include <queue>

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;
			class DescriptorPoolPage_Vulkan;

			class RHI_DescriptorLayout_Vulkan : public RHI_DescriptorLayout
			{
			public:
				VkDescriptorSetLayout GetLayout() const { return m_layout; }

				/// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::string name) override;

			protected:
				virtual void Create(RenderContext* context, int set, DescriptorSet descriptor_sets) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				VkDescriptorSetLayout m_layout;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)