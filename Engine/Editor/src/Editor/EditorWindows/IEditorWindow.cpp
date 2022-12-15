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
			if (ImGui::Begin(GetWindowName(), &m_isOpen))
			{
				OnDraw();
				ImVec2 windowSize = ImGui::GetWindowSize();
				m_width = windowSize.x;
				m_height = windowSize.y;

				if (!m_isOpen)
				{
					EditorWindowManager::Instance().RemoveWindow(GetWindowName());
				}
			}
			ImGui::End();
		}
	}
}