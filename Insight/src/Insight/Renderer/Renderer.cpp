#include "ispch.h"
#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/OpenGL/OpenGLRenderer.h"
#endif

#include "Insight/Renderer/Renderer.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Config/Config.h"

namespace Insight
{
	GraphicsAPI Renderer::s_API = GraphicsAPI::Vulkan;

	Renderer* Renderer::Create()
	{
		switch (s_API)
		{
		case GraphicsAPI::Vulkan: return NEW_ON_HEAP(vks::VulkanRenderer);
		case GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLRenderer);
		}
		return nullptr;
	}
}