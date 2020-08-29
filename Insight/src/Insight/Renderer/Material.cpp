#include "ispch.h"
#include "Material.h"

#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanMaterial.h"
typedef Platform::VulkanMaterial PlatformMaterial;
#elif defined(IS_OPENGL)
#include "Insight/Renderer/OpenGL/OpenGLMaterial.h"
typedef Insight::Render::OpenGLMaterial PlatformMaterial;
#endif

Material* Material::Create()
{
	return NEW_ON_HEAP(PlatformMaterial);
}
