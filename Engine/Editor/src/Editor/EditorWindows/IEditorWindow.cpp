#include "Editor/EditorWindows/IEditorWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "imgui/imgui.h"

namespace Insight
{
	namespace Editor
	{
		IEditorWindow::IEditorWindow()
		{ }

		IEditorWindow::IEditorWindow(u32 minWidth, u32 minHeight)
			: m_minWidth(minWidth)
			, m_minHeight(minHeight)
			, m_maxWidth(-1)
			, m_maxHeight(-1)
		{ }

		IEditorWindow::IEditorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: m_minWidth(minWidth)
			, m_minHeight(minHeight)
			, m_maxWidth(maxWidth)
			, m_maxHeight(maxHeight)
		{ }

		IEditorWindow::~IEditorWindow()
		{ }

		void IEditorWindow::Draw()
		{
			const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
			if (m_isFullscreen)
			{
				if (ImGui::Begin(GetWindowName(), &m_isOpen, windowFlags))
				{
					const ImGuiIO& io = ImGui::GetIO();
					ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
					ImGui::SetWindowSize(ImVec2(static_cast<float>(io.DisplaySize.x), static_cast<float>(io.DisplaySize.y)));

					OnDraw();
					if (!m_isOpen)
					{
						EditorWindowManager::Instance().RemoveWindow(GetWindowName());
					}
				}
			}
			else
			{
				if (ImGui::Begin(GetWindowName(), &m_isOpen, windowFlags))
				{
					OnDraw();

					ImVec2 windowPosition = ImGui::GetWindowPos();
					m_positionX = windowPosition.x;
					m_positionY = windowPosition.y;
					ImVec2 windowSize = ImGui::GetWindowSize();
					m_width = static_cast<u32>(windowSize.x);
					m_height = static_cast<u32>(windowSize.y);

					if (!m_isOpen)
					{
						EditorWindowManager::Instance().RemoveWindow(GetWindowName());
					}
				}
			}
			ImGui::End();
		}

		void IEditorWindow::SetFullscreen(bool value)
		{
			m_isFullscreen = value;
		}

		bool IEditorWindow::GetFullscreen() const
		{
			return m_isFullscreen;
		}
	}
}