#include "ispch.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"

#endif

#include "Engine/Graphics/Renderer.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Config/Config.h"

GraphicsRendererAPI Renderer::s_API = GraphicsRendererAPI::Vulkan;

Renderer* Renderer::New()
{
	switch (s_API)
	{
		case GraphicsRendererAPI::Vulkan: return ::New<vks::VulkanRenderer>();
		//case GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLRenderer);
	}
	return nullptr;
}