#pragma once

#include "Graphics/PipelineStateObject.h"
#include "Threading/SpinLock.h"

#include <map>
#include <array>


namespace Insight
{
	namespace Graphics
	{
		class RenderContext;
		class RHI_Pipeline;
		class RHI_PipelineLayout;

		class RHI_PipelineCahce : public Core::Singleton<RHI_PipelineCahce>
		{
		public:
			virtual ~RHI_PipelineCahce() { }

			static RHI_PipelineCahce* New(RenderContext* context);

			virtual RHI_Pipeline* GetGraphicsPSO(PipelineStateObject& pso) const = 0;
			virtual void AddPSO(const PipelineStateObject& pso, const RHI_Pipeline* pipeline) = 0;
		};

		class RHI_PipelineLayoutManager
		{
		public:
			RHI_PipelineLayoutManager();
			~RHI_PipelineLayoutManager();

			void SetRenderContext(RenderContext* context);
			RHI_PipelineLayout* GetOrCreateLayout(PipelineStateObject pso);
			RHI_PipelineLayout* GetOrCreateLayout(ComputePipelineStateObject pso);
			void Destroy();

		private:
			u64 HashDescriptors(const RHI_Shader* shader) const;

		private:
			std::map<u64, RHI_PipelineLayout*> m_layouts;
			RenderContext* m_context = nullptr;
			Threading::SpinLock m_lock;
		};

		class RHI_PipelineManager
		{
		public:
			RHI_PipelineManager();
			~RHI_PipelineManager();

			void SetRenderContext(RenderContext* context);
			RHI_Pipeline* GetOrCreatePSO(PipelineStateObject pso);
			RHI_Pipeline* GetOrCreatePSO(ComputePipelineStateObject pso);

			// Should be called before use, as this can allow the system to create single/multiple pso
			// in parallel and before use to reduce stuttering.
			IS_GRAPHICS void PreWawmPSO(PipelineStateObject pso);
			IS_GRAPHICS void CreatePreWarmPSO();

			void Destroy();

			void DestroyPipelineWithShader(const ShaderDesc& shaderDesc);

		private:
			std::vector<std::pair<RHI_Pipeline*, PipelineStateObject>> m_preWarmPsos;
			std::map<u64, RHI_Pipeline*> m_pipelineStateObjects;

			//RHI_PipelineCahce* m_pipelineCache = nullptr;
			RenderContext* m_context = nullptr;

			Threading::SpinLock m_lock;
		};
	}
}