#include "ispch.h"
#include "Engine/Graphics/Material.h"
#include "Engine/Module/GraphicsModule.h"
#include "Platform/Vulkan/VulkanMaterial.h"

Material::Material()
{
}

Material* Material::New()
{
	switch (Module::GraphicsModule::Instance()->GetAPI())
	{
	case GraphicsRendererAPI::Vulkan: return ::New<vks::VulkanMaterial>();
		//case GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLMaterial);
	}
	return nullptr;
}