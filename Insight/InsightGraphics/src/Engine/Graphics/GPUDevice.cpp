
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Graphics/Image/GPUImage.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/Graphics/Graphics.h"

namespace Insight::Graphics
{
	GPUDevice* GPUDevice::New()
	{
		if (::Graphics::IsVulkan())
		{
			return ::New<GraphicsAPI::Vulkan::GPUDeviceVulkan>();
		}
		ASSERT(false && "[GPUDevice::New] API not supported.");
		return nullptr;
	}

	GPUDevice::GPUDevice(RendererType rendererType, ShaderProfile shaderProfile)
		: m_state(DeviceState::Missing)
		, m_isRendering(false)
		, m_featureLevel()
		, m_rendererType(rendererType)
		, m_shaderProfile(shaderProfile)
		, Resources(1024)
		, m_totalGraphicsMemory(0)
	{
	}

	GPUDevice::~GPUDevice()
	{
		ASSERT(Instance() == this);
	}

	bool GPUDevice::LoadContent()
	{
		return false;
	}

	void GPUDevice::Dispose()
	{
	}
}