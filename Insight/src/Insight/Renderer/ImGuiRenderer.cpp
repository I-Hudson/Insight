#include "ispch.h"
#include "Insight/Renderer/ImGuiRenderer.h"

#include "Platform/Vulkan/VulkanImGUIRenderer.h"

namespace Insight
{
	ImGuiRenderer::ImGuiRenderer()
	{
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
	}

	SharedPtr<ImGuiRenderer> Insight::ImGuiRenderer::Create()
	{
		switch (Renderer::s_API)
		{
			case GraphicsAPI::Vulkan: return CreateSharedPtr<vks::VulkanImGUIRenderer>();
		}

		return nullptr;
	}
}