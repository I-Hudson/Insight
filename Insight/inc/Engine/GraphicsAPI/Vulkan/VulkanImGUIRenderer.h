#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "Engine/GraphicsAPI/Vulkan/VulkanHeaders.h"
#include <imgui.h>
#include <glm/glm.hpp>

class Event;

namespace Insight::GraphicsAPI::Vulkan
{
	class VulkanImGUIRenderer : public ImGuiRenderer
	{
	public:
		VulkanImGUIRenderer();
		virtual ~VulkanImGUIRenderer() override;

		virtual void NewFrame() override;
		virtual void EndFrame() override;
		virtual void Render() override;

	private:
		virtual void Init() override;
		void InitResources();
		void SetupImGuiRenderPass();
		void SetupImGuiDescriptorPool();
		void SetStyle();

		void WindowResize(Event const& event);

	private:
		bool m_newFrameInit = false;

		VkDescriptorPool m_descriptorPool;
		VkRenderPass m_renderpass;
	};
}