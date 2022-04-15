#pragma once

#include "Graphics/GPU/GPUPipelineStateObject.h"
#include "Graphics/GPU/RHI/Vulkan/GPUDevice_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		namespace RHI::Vulkan
		{
			class GPUPipelineStateObject_Vulkan : public GPUPipelineStateObject, public GPUResource_Vulkan
			{
			public:
				GPUPipelineStateObject_Vulkan();
				virtual ~GPUPipelineStateObject_Vulkan() override;

				PipelineStateObject GetPSO() const { return m_pso; }
				vk::Pipeline GetPipeline() const { return m_pipeline; }

			protected:
				virtual void Create(PipelineStateObject pso) override;
				virtual void Destroy() override;

			private:
				PipelineStateObject m_pso;
				vk::Pipeline m_pipeline{ nullptr };
				vk::PipelineLayout m_pipelineLayout{ nullptr };
			};

			struct GPURenderpassDesc_Vulkan
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

			class GPURenderpassManager_Vulkan : public GPUResource_Vulkan
			{
			public:
				~GPURenderpassManager_Vulkan();

				static GPURenderpassManager_Vulkan& Instance()
				{
					static GPURenderpassManager_Vulkan ins;
					return ins;
				}
				
				vk::RenderPass GetOrCreateRenderpass(GPURenderpassDesc_Vulkan desc);
				void Destroy();

			private:
				std::map<u64, vk::RenderPass> m_renderpasses;
			};
		}
	}
}