#include "ispch.h"
#include "Insight/Renderer/ImGuiRenderer.h"
#include "Module/GraphicsModule.h"
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
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsAPI::Vulkan: return CreateSharedPtr<vks::VulkanImGUIRenderer>();
		}

		return nullptr;
	}
}