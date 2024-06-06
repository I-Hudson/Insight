#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_Pipeline.h"

#include <d3d12.h>
#include <d3dx12.h>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            class RenderContext_DX12;

            class RHI_Pipeline_DX12 : public RHI_Pipeline
            {
            public:
                virtual ~RHI_Pipeline_DX12() override;

                ID3D12PipelineState* GetPipeline() const;

                // RHI_Pipeline
                virtual void Create(RenderContext* context, PipelineStateObject pso) override;
                virtual void Create(RenderContext* context, ComputePipelineStateObject pso) override;

                // RHI_Resource
                virtual void Release() override;
                virtual bool ValidResource() override;
                virtual void SetName(std::string name) override;

            private:
                ID3D12PipelineState* m_pipeline = nullptr;
                RenderContext_DX12* m_context;
            };
        }
    }
}
#endif // IS_VULKAN_ENABLED