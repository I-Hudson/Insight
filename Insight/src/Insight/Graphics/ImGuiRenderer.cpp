#include "ispch.h"
#include "Graphics/ImGuiRenderer.h"
#include "Module/GraphicsModule.h"
#include "Vulkan/VulkanImGUIRenderer.h"

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