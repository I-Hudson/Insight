#include "Graphics/RHI/RHI_Sampler.h"
#include "Graphics/RenderContext.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"
#endif
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RHI_Sampler_DX12.h"
#endif

namespace Insight
{
	namespace Graphics
	{
		RHI_SamplerManager* RHI_SamplerManager::RHI_SamplerManager::New()
		{
#ifdef IS_VULKAN_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return ::New<RHI::Vulkan::RHI_SamplerManager_Vulkan, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
#ifdef IS_DX12_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12) { return ::New<RHI::DX12::RHI_SamplerManager_DX12, Insight::Core::MemoryAllocCategory::Graphics>(); }
#endif
			ASSERT(false);
			return nullptr;
		}
	}
}