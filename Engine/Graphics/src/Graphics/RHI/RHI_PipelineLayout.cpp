#include "Graphics/RHI/RHI_PipelineLayout.h"

#include "Graphics/RenderContext.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RHI_PipelineLayout_Vulkan.h"
#endif
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RHI_PipelineLayout_DX12.h"
#endif

namespace Insight
{
    namespace Graphics
    {
        RHI_PipelineLayout* RHI_PipelineLayout::New()
        {
#if defined(IS_VULKAN_ENABLED)
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return NewTracked(RHI::Vulkan::RHI_PipelineLayout_Vulkan); }
#endif
#if defined(IS_DX12_ENABLED)
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return NewTracked(RHI::DX12::RHI_PipelineLayout_DX12); }
#endif
            FAIL_ASSERT();
            return nullptr;
        }
    }
}