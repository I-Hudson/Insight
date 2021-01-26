#include "ispch.h"
#include "GPUDevice.h"
#include "GPUBuffer.h"
#include "Material.h"
#include "Engine/GraphicsAPI/Vulkan/GPUDeviceVulkan.h"
#include "Engine/Module/GraphicsModule.h"

struct GPUDevice::DefaultData
{
	Material* FullScreenMaterial;
	GPUBuffer* FullscreenTriangle;
};

GPUDevice::GPUDevice(RendererType rendererType, ShaderProfile shaderProfile)
	: m_state(DeviceState::Missing)
	, m_isRendering(false)
	, m_featureLevel()
	, m_defaultData(::New<DefaultData>())
	, m_rendererType(rendererType)
	, m_shaderProfile(shaderProfile)
	, m_collection(1024)
	, m_totalGraphicsMemory(0)
{
}

GPUDevice::~GPUDevice()
{
	::Delete(m_defaultData);

	ASSERT(Instance() == this);
}

bool GPUDevice::Init()
{
	return false;
}

bool GPUDevice::LoadContent()
{
	m_defaultData->FullScreenMaterial = Material::New();
	m_defaultData->FullScreenMaterial->CreateDefault();

	return false;
}

void GPUDevice::Dispose()
{
}

void GPUDevice::WaitForGPU()
{
}
