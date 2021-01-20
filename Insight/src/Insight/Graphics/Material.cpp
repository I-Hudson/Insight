#include "ispch.h"
#include "Material.h"

#include "Module/GraphicsModule.h"
#include "Platform/Vulkan/VulkanMaterial.h"

SharedPtr<Material> Material::Create()
{
	switch (Insight::Module::GraphicsModule::Instance()->GetAPI())
	{
	case Insight::GraphicsAPI::Vulkan: return Object::CreateObject<vks::VulkanMaterial>();
		//case Insight::GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLMaterial);
	}
	return SharedPtr<Material>();
}
