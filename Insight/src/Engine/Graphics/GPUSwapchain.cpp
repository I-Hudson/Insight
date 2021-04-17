#include "ispch.h"
#include "Engine/Graphics/GPUSwapchain.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSwapchainVulkan.h"

namespace Insight::Graphics
{
	GPUSwapchain* GPUSwapchain::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUSwapchainVulkan>();
		}
		ASSERT(false && "[GPUSwapchain::New] API not supported.");
		return nullptr;
	}
}