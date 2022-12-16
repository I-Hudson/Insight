#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/PipelineStateObject.h"

#include <vulkan/vulkan_core.h>

#include <map>
#include <array>


namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class RenderContext_Vulkan;

			class PipelineLayoutManager_Vulkan
			{
			public:
				PipelineLayoutManager_Vulkan();
				~PipelineLayoutManager_Vulkan();

				void SetRenderContext(RenderContext_Vulkan* context) { m_context = context; }
				VkPipelineLayout GetOrCreateLayout(PipelineStateObject pso);
				void Destroy();

			private:
				std::map<u64, VkPipelineLayout> m_layouts;
				RenderContext_Vulkan* m_context{ nullptr };
			};

			class PipelineStateObjectManager_Vulkan
			{
			public:
				PipelineStateObjectManager_Vulkan();
				~PipelineStateObjectManager_Vulkan();

				void SetRenderContext(RenderContext_Vulkan* context) { m_context = context; }
				VkPipeline GetOrCreatePSO(PipelineStateObject pso);
				void Destroy();

			private:
				std::map<u64, VkPipeline> m_pipelineStateObjects;
				RenderContext_Vulkan* m_context{ nullptr };
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)