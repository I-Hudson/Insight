#include "ispch.h"
#include "Insight/Renderer/Renderer.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Config/Config.h"

#include "Insight/Renderer/VulkanRenderer.h"

namespace Insight
{
	Renderer* Renderer::Create(RendererStartUpData& startupData)
	{
		Renderer* renderer = nullptr;
		switch ((GraphicsAPI)CONFIG_VAL(Config::GraphicsConfig.GraphicsAPI))
		{
			case GraphicsAPI::Vulkan:
				renderer = Memory::MemoryManager::NewOnFreeList<Render::VulkanRenderer>(startupData);
				break;
		}

		return renderer;
	}
}