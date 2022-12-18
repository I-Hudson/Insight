#pragma once

#if defined(IS_VULKAN_ENABLED)

#include "Graphics/RHI/RHI_PipelineLayout.h"

#include <vulkan/vulkan_core.h>

namespace Insight
{
    namespace Graphics
    {
        namespace RHI::Vulkan
        {
            class RenderContext_Vulkan;

            class RHI_PipelineLayout_Vulkan : public RHI_PipelineLayout
            {
            public:
                virtual ~RHI_PipelineLayout_Vulkan() override;

                VkPipelineLayout GetPipelineLayout() const;

                // RHI_Pipeline
                virtual void Create(RenderContext* context, PipelineStateObject pso) override;

                // RHI_Resource
                virtual void Release() override;
                virtual bool ValidResource() override;
                virtual void SetName(std::string name) override;

            private:
                VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
                RenderContext_Vulkan* m_context;
            };
        }
    }
}
#endif // IS_VULKAN_ENABLED