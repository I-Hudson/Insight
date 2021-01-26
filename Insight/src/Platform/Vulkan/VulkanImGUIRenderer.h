#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Graphics/ImGuiRenderer.h"
#include "VulkanHeader.h"
#include "VulkanBuffer.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include "Engine/Threading/TThreadSafe.h"

	class Event;

namespace vks
{
	class VulkanImGUIRenderer : public ImGuiRenderer
	{
	public:
		VulkanImGUIRenderer();
		virtual ~VulkanImGUIRenderer() override;

		virtual void NewFrame() override;
		virtual void EndFrame() override;
		void Render(VkCommandBuffer commandBuffer);

	private:
		virtual void Init(Renderer* renderer) override;
		void InitResources();
		void SetupImGuiRenderPass();
		void SetStyle();

		void WindowResize(Event const& event);

	private:
		bool m_newFrameInit = false;
	};
}