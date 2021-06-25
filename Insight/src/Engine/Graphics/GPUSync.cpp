
#include "Engine/Graphics/GPUSync.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"

namespace Insight::Graphics
{
	GPUFence* GPUFence::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUFenceVulkan>();
		}
		ASSERT(false && "[GPUFence::New] API not supported.");
		return nullptr;
	}

	GPUSemaphore* GPUSemaphore::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUSemaphoreVulkan>();
		}
		ASSERT(false && "[GPUFence::New] API not supported.");
		return nullptr;
	}
}
