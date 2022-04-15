#pragma once

#include "Graphics/PipelineStateObject.h"
#include "vulkan/vulkan.hpp"
#include <map>

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
				vk::PipelineLayout GetOrCreateLayout();
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

			struct RenderpassDesc_Vulkan
			{
				std::vector<RenderTarget*> RenderTargets;
			
				u64 GetHash() const
				{
					u64 hash = 0;
			
					for (const auto* rt : RenderTargets)
					{
						HashCombine(hash, rt);
					}
			
					return hash;
				}
			};
			
			class RenderpassManager_Vulkan
			{
			public:
				~RenderpassManager_Vulkan();
			
				void SetRenderContext(RenderContext_Vulkan* context) { m_context = context; }
				vk::RenderPass GetOrCreateRenderpass(RenderpassDesc_Vulkan desc);
				void Destroy();
			
			private:
				std::map<u64, vk::RenderPass> m_renderpasses;
				RenderContext_Vulkan* m_context{ nullptr };
			};
		}
	}
}