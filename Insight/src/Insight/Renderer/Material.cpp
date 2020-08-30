#include "ispch.h"
#include "Material.h"

#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanMaterial.h"
typedef Platform::VulkanMaterial PlatformMaterial;
#elif defined(IS_OPENGL)
//#include "Insight/Renderer/OpenGL/OpenGLMaterial.h"
//typedef Insight::Render::OpenGLMaterial PlatformMaterial;
#endif

Material* Material::Create()
{
#if defined(IS_VULKAN) && !defined(IS_OPENGL)
	return NEW_ON_HEAP(PlatformMaterial);
#elif defined(IS_OPENGL) && !defined(IS_VULKAN)
	IS_IMPLERMENT("Opengl material missing.");
	return nullptr;
#else
	return nullptr;
#endif
}
