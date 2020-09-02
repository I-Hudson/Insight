#include "ispch.h"
#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"
#elif defined(IS_OPENGL)
#include "Platform/OpenGL/OpenGLRenderer.h"
#endif

#include "Insight/Renderer/Renderer.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Config/Config.h"

namespace Insight
{
	Renderer* Renderer::Create()
	{
#if defined(IS_VULKAN) && !defined(IS_OPENGL)
		return NEW_ON_HEAP(Platform::VulkanRenderer);
#elif defined(IS_OPENGL) && !defined(IS_VULKAN)
		return NEW_ON_HEAP(Platform::OpenGLRenderer);
#else
		return nullptr;
#endif
	}
}