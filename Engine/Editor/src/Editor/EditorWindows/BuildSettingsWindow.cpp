#include "Editor/EditorWindows/BuildSettingsWindow.h"

namespace Insight
{
	namespace Editor
	{
		BuildSettingsWindow::BuildSettingsWindow()
			: IEditorWindow()
		{ }

		BuildSettingsWindow::BuildSettingsWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		BuildSettingsWindow::BuildSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		BuildSettingsWindow::~BuildSettingsWindow()
		{ }

		void BuildSettingsWindow::OnDraw()
		{
		}
	}
}