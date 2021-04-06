#include "ispch.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/Module/GraphicsModule.h"
#include "Platform/Vulkan/VulkanImGUIRenderer.h"

ImGuiRenderer::ImGuiRenderer()
{
}

ImGuiRenderer::~ImGuiRenderer()
{
}

ImGuiRenderer* ImGuiRenderer::New()
{
	switch (Module::GraphicsModule::Instance()->GetAPI())
	{
	case GraphicsRendererAPI::Vulkan: return ::New<vks::VulkanImGUIRenderer>();
	}

	return nullptr;
}