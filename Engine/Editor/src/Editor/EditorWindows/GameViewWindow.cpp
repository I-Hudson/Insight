#include "Editor/EditorWindows/GameViewWindow.h"

#include "Graphics/RenderGraph/RenderGraph.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		GameViewWindow::GameViewWindow()
			: IEditorWindow()
		{ }

		GameViewWindow::GameViewWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		GameViewWindow::GameViewWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		GameViewWindow::~GameViewWindow()
		{ }

		void GameViewWindow::Initialise()
		{
			m_imguiWindowFlags = 0
				| ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoCollapse;
		}

		void GameViewWindow::OnDraw()
		{
			Graphics::RHI_Texture* gameViewTexture = Graphics::RenderGraph::Instance().GetRenderCompletedRHITexture("Composite_Tex");
			if (gameViewTexture == nullptr)
			{
				return;
			}
			ImVec2 windowSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			ImGui::Image(gameViewTexture, windowSize);
		}
	}
}