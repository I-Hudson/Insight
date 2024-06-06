#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_Pipeline.h"

#include <vulkan/vulkan_core.h>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::Vulkan
        {
            class RenderContext_Vulkan;

            class RHI_Pipeline_Vulkan : public RHI_Pipeline
            {
            public:
                virtual ~RHI_Pipeline_Vulkan() override;

                VkPipeline GetPipeline() const;

                // RHI_Pipeline
                virtual void Create(RenderContext* context, PipelineStateObject pso) override;
                virtual void Create(RenderContext* context, ComputePipelineStateObject pso) override;

                // RHI_Resource
                virtual void Release() override;
                virtual bool ValidResource() override;
                virtual void SetName(std::string name) override;

            private:
                VkPipeline m_pipeline = VK_NULL_HANDLE;
                RenderContext_Vulkan* m_context;
            };
        }
    }
}
#endif // IS_VULKAN_ENABLED