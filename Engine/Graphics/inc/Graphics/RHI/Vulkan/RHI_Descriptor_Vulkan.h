#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/PipelineStateObject.h"

#include <unordered_set>
#include <queue>

#include <vulkan/vulkan.hpp>

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
				vk::DescriptorSetLayout GetLayout() const { return m_layout; }

				/// RHI_Resouce
				virtual void Release() override;
				virtual bool ValidResouce() override;
				virtual void SetName(std::wstring name) override;

			protected:
				virtual void Create(RenderContext* context, int set, DescriptorSet descriptor_sets) override;

			private:
				RenderContext_Vulkan* m_context{ nullptr };
				vk::DescriptorSetLayout m_layout;
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)