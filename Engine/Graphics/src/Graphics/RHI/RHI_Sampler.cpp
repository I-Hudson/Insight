#include "Graphics/RHI/RHI_Sampler.h"
#include "Graphics/RenderContext.h"

#ifdef IS_VULKAN_ENABLED
#include "Graphics/RHI/Vulkan/RHI_Sampler_Vulkan.h"
#endif
#ifdef IS_DX12_ENABLED
#endif

namespace Insight
{
	namespace Graphics
	{
		RHI_SamplerManager* RHI_SamplerManager::RHI_SamplerManager::New()
		{
#ifdef IS_VULKAN_ENABLED
			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::Vulkan) { return NewTracked(RHI::Vulkan::RHI_SamplerManager_Vulkan); }
#endif
#ifdef IS_DX12_ENABLED
			if (GraphicsManager::IsDX12()) { }
#endif
			ASSERT(false);
			return nullptr;
		}
	}
}