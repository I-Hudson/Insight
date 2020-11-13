#include "ispch.h"
#include "Insight/Renderer/ImGuiRenderer.h"

#include "Platform/Vulkan/VulkanImGUIRenderer.h"

namespace Insight
{
	ImGuiRenderer::ImGuiRenderer()
	{
		SetInstancePtr(this);
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
		ClearPtr();
	}

	ImGuiRenderer* Insight::ImGuiRenderer::Create()
	{
		switch (Renderer::s_API)
		{
			case GraphicsAPI::Vulkan: return NEW_ON_HEAP(vks::VulkanImGUIRenderer);
		}

		return nullptr;
	}
}