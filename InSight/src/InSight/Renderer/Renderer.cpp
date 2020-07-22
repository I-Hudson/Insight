#include "ispch.h"

#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/VulkanRenderer.h"
typedef Insight::Render::VulkanRenderer PlatformRenderer;
#elif defined(IS_OPENGL)
#include "Insight/Renderer/OpenGL/OpenGLRenderer.h"
typedef Insight::Render::OpenGLRenderer PlatformRenderer;
#endif

#include "Insight/Renderer/Renderer.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Config/Config.h"

namespace Insight
{
	Renderer* Renderer::Create(RendererStartUpData& startupData)
	{
		return Memory::MemoryManager::NewOnFreeList<PlatformRenderer>(startupData);
	}
}