#include "ispch.h"
#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"
typedef Platform::VulkanRenderer PlatformRenderer;
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
		return NEW_ON_HEAP(PlatformRenderer);
	}
}