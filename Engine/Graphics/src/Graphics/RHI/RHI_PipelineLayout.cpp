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
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_PipelineLayout_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#if defined(IS_DX12_ENABLED)
            if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_PipelineLayout_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
            FAIL_ASSERT();
            return nullptr;
        }
    }
}