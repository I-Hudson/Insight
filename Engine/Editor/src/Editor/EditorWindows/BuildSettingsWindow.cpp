#include "Editor/EditorWindows/BuildSettingsWindow.h"
#include "Editor/PackageBuild.h"

#include "Platforms/Platform.h"
#include "Editor/Build/BuildSystem.h"
#include "Runtime/Engine.h"
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

		void BuildSettingsWindow::Initialise()
		{
			m_buildSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<BuildSystem>();
			ASSERT(m_buildSystem);
		}

		void BuildSettingsWindow::Shutdown()
		{
			m_buildSystem = nullptr;
		}

		void BuildSettingsWindow::OnDraw()
		{
			IS_PROFILE_FUNCTION();
		
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