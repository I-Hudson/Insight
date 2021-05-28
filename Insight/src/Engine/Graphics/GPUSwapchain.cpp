#include "ispch.h"
#include "Engine/Graphics/GPUSwapchain.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSwapchainVulkan.h"

namespace Insight::Graphics
{
	GPUSwapchain* GPUSwapchain::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUSwapchainVulkan>();
		}
		ASSERT(false && "[GPUSwapchain::New] API not supported.");
		return nullptr;
	}
}