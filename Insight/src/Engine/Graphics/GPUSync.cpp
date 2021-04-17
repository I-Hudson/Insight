#include "ispch.h"
#include "Engine/Graphics/GPUSync.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUSyncVulkan.h"

namespace Insight::Graphics
{
	GPUFence* GPUFence::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUFenceVulkan>();
		}
		ASSERT(false && "[GPUFence::New] API not supported.");
		return nullptr;
	}

	GPUSemaphore* GPUSemaphore::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUSemaphoreVulkan>();
		}
		ASSERT(false && "[GPUFence::New] API not supported.");
		return nullptr;
	}
}
