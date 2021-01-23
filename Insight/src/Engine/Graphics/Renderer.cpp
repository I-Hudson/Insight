#include "ispch.h"
#include "Engine/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Platform/Vulkan/VulkanRenderer.h"

#endif

#include "Engine/Graphics/Renderer.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Config/Config.h"

	GraphicsAPI Renderer::s_API = GraphicsAPI::Vulkan;

	SharedPtr<Renderer> Renderer::Create()
	{
		switch (s_API)
		{
		case GraphicsAPI::Vulkan: return Object::CreateObject<vks::VulkanRenderer>();
		//case GraphicsAPI::OpenGL: return NEW_ON_HEAP(Platform::OpenGLRenderer);
		}
		return nullptr;
	}