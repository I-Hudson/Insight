#pragma once

#if defined(IS_DX12_ENABLED)

#include "Graphics/RHI/RHI_PipelineLayout.h"

#include <d3dx12.h>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::DX12
        {
            class RenderContext_DX12;

            class RHI_PipelineLayout_DX12 : public RHI_PipelineLayout
            {
            public:
                virtual ~RHI_PipelineLayout_DX12() override;

                ID3D12RootSignature* GetRootSignature() const;

                // RHI_Pipeline
                virtual void Create(RenderContext* context, PipelineStateObject pso) override;

                // RHI_Resource
                virtual void Release() override;
                virtual bool ValidResource() override;
                virtual void SetName(std::string name) override;

            private:
                ID3D12RootSignature* m_rootSignature = nullptr;
                RenderContext_DX12* m_context;
            };
        }
    }
}
#endif // IS_VULKAN_ENABLED