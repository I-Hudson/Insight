
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/Graphics/Graphics.h"
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
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDynamicBufferVulkan>();
		}

		ASSERT(false && "[GPUDynamicBuffer::New] API not supported.");
		return nullptr;
	}
}