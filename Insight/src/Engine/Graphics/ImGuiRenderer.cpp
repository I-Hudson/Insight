
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanImGUIRenderer.h"

ImGuiRenderer::ImGuiRenderer()
{
}

ImGuiRenderer::~ImGuiRenderer()
{
}

ImGuiRenderer* ImGuiRenderer::New()
{
	if (::Graphics::IsVulkan())
	{
		return ::New<Insight::GraphicsAPI::Vulkan::VulkanImGUIRenderer>();
	}

	return nullptr;
}