#pragma once

#include "Graphics/PipelineStateObject.h"

#include <map>
#include <array>


namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Pipeline;
		class RHI_PipelineLayout;

		class RHI_PipelineLayoutManager
		{
		public:
			RHI_PipelineLayoutManager();
			~RHI_PipelineLayoutManager();

			void SetRenderContext(RenderContext* context);
			RHI_PipelineLayout* GetOrCreateLayout(PipelineStateObject pso);
			void Destroy();

		private:
			std::map<u64, RHI_PipelineLayout*> m_layouts;
			RenderContext* m_context = nullptr;
		};

		class RHI_PipelineManager
		{
		public:
			RHI_PipelineManager();
			~RHI_PipelineManager();

			void SetRenderContext(RenderContext* context);
			RHI_Pipeline* GetOrCreatePSO(PipelineStateObject pso);
			void Destroy();

		private:
			std::map<u64, RHI_Pipeline*> m_pipelineStateObjects;
			RenderContext* m_context = nullptr;
		};
	}
}