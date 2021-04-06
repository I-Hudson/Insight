#include "ispch.h"
#include "Engine/GraphicsAPI/Vulkan/GPUCommandBufferVulkan.h"

namespace Insight::GraphicsAPI::Vulkan
{
	GPUCommandBufferVulkan::GPUCommandBufferVulkan()
		: m_cmdBuffer(nullptr)
	{
	}

	GPUCommandBufferVulkan::~GPUCommandBufferVulkan()
	{
	}

	void GPUCommandBufferVulkan::Init(Graphics::GPUCommandBufferDesc const& desc)
	{
	}

	void GPUCommandBufferVulkan::BeginRecord()
	{
	}

	void GPUCommandBufferVulkan::EndRecord()
	{
	}

	void GPUCommandBufferVulkan::Submit()
	{
	}

	void GPUCommandBufferVulkan::Clear()
	{
	}

	void GPUCommandBufferVulkan::OnReleaseGPU()
	{
	}
}