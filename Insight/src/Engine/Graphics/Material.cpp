#include "ispch.h"
#include "Material.h"

#include "Engine/Module/GraphicsModule.h"
#include "Platform/Vulkan/VulkanMaterial.h"

SharedPtr<Material> Material::Create()
{
	switch (Module::GraphicsModule::Instance()->GetAPI())
	{
	case GraphicsAPI::Vulkan: return Object::CreateObject<vks::VulkanMaterial>();
		//case GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLMaterial);
	}
	return SharedPtr<Material>();
}
