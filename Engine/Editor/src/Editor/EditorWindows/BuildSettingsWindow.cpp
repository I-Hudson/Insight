#include "Editor/EditorWindows/BuildSettingsWindow.h"
#include "Editor/PackageBuild.h"

#include "Platforms/Platform.h"

#include <imgui.h>

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
			if (ImGui::Button("Package Build"))
			{
				std::string folderSelcted;
				PlatformFileDialog fileDialog;
				if (fileDialog.Show(PlatformFileDialogOperations::SelectFolder, &folderSelcted))
				{
					PackageBuild build;
					build.Build(folderSelcted);
				}
			}
		}
	}
}