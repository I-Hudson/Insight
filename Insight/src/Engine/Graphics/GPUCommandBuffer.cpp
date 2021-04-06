#include "ispch.h"
#include "Engine/Graphics/GPUCommandBuffer.h"

#include "Engine/Module/GraphicsModule.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"

namespace Insight::Graphics
{
	GPUCommandBuffer* GPUCommandBuffer::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUCommandBufferVulkan>();
		}

		ASSERT(false && "[GPUCommandBuffer::New] API not supported.");
	}
}
