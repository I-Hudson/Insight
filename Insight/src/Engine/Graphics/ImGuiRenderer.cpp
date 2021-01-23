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

	SharedPtr<ImGuiRenderer> ImGuiRenderer::Create()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsAPI::Vulkan: return CreateSharedPtr<vks::VulkanImGUIRenderer>();
		}

		return nullptr;
	}