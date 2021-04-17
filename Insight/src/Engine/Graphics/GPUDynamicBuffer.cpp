#include "ispch.h"
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDynamicBufferVulkan.h"

namespace Insight::Graphics
{
	GPUDynamicBuffer::GPUDynamicBuffer()
	{
	}

	GPUDynamicBuffer::~GPUDynamicBuffer()
	{
		Reset();
	}

	GPUDynamicBuffer* GPUDynamicBuffer::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUDynamicBufferVulkan>();
		}

		ASSERT(false && "[GPUDynamicBuffer::New] API not supported.");
		return nullptr;
	}
}