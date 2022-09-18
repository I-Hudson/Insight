#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/PipelineStateObject.h"
#include <map>
#include <array>

/// Try and reduce compile time with not including vulkan.hpp in this header.
namespace vk
{
	class Pipeline;
	class PipelineLayout;
	class RenderPass;
}

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
				vk::PipelineLayout GetOrCreateLayout(PipelineStateObject pso);
				void Destroy();

			private:
				std::map<u64, vk::PipelineLayout> m_layouts;
				RenderContext_Vulkan* m_context{ nullptr };
			};

			class PipelineStateObjectManager_Vulkan
			{
			public:
				PipelineStateObjectManager_Vulkan();
				~PipelineStateObjectManager_Vulkan();

				void SetRenderContext(RenderContext_Vulkan* context) { m_context = context; }
				vk::Pipeline GetOrCreatePSO(PipelineStateObject pso);
				void Destroy();

			private:
				std::map<u64, vk::Pipeline> m_pipelineStateObjects;
				RenderContext_Vulkan* m_context{ nullptr };
			};
		}
	}
}

#endif ///#if defined(IS_VULKAN_ENABLED)