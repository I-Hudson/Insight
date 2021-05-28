#include "ispch.h"
#include "Engine/Graphics/GPUCommandBuffer.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"

namespace Insight::Graphics
{
	GPUCommandBuffer::GPUCommandBuffer()
		: m_recordCommandCount(0)
		, m_state(GPUCommandBufferState::IDLE)
		, m_desc(GPUCommandBufferDesc(GPUCommandBufferUsageFlags::INVALID))
	{ }

	GPUCommandBuffer::~GPUCommandBuffer()
	{ }

	GPUCommandBuffer* GPUCommandBuffer::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUCommandBufferVulkan>();
		}
		ASSERT(false && "[GPUCommandBuffer::New] API not supported.");
		return nullptr;
	}

	GPUCommandPool::GPUCommandPool()
		: m_desc(GPUCommandPoolDesc(GPUCommandPoolFlags::INVALID, GPUQueue::GRAPHICS))
	{ }

	GPUCommandPool::~GPUCommandPool()
	{
		ASSERT(m_buffers.size() == 0 && "[GPUCommandPool::~GPUCommandPool] Command buffers have not been freed.");
	}

	GPUCommandPool* GPUCommandPool::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUCommandPoolVulkan>();
		}

		ASSERT(false && "[GPUCommandPool::New] API not supported.");
		return nullptr;
	}
}
