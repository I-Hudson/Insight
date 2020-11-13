#include "ispch.h"
#include "Material.h"

#include "Insight/Memory/MemoryManager.h"
#include "Renderer.h"

#ifdef IS_VULKAN
//#include "Platform/Vulkan/VulkanMaterial.h"
#endif
#include "Platform/OpenGL/OpenGLMaterial.h"

Material* Material::Create()
{
	switch (Insight::Renderer::s_API)
	{
#if defined(IS_VULKAN)
	//case Insight::GraphicsAPI::Vulkan: return NEW_ON_HEAP(Platform::VulkanMaterial);
#endif
	case Insight::GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLMaterial);
	}
	return nullptr;
}
