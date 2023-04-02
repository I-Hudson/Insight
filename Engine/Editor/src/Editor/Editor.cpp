#include "Editor/Editor.h"

#include "EditorModule.h"
#include "Editor/EditorWindows/ProjectWindow.h"

#include "Runtime/EntryPoint.h"

#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		void Editor::OnInit()
		{
			EditorModule::Initialise(GetSystemRegistry().GetSystem<Core::ImGuiSystem>());

			std::string windowTitle = "Insight Editor";
#ifdef IS_DEBUG
			windowTitle += " Debug ";
#elif IS_RELEASE
			windowTitle += " Release ";
#endif
			windowTitle += "(";
			windowTitle += Graphics::GraphicsAPIToString(Graphics::RenderContext::Instance().GetGraphicsAPI());
			windowTitle += ")";

			Graphics::Window::Instance().SetTite(windowTitle);
			Graphics::Window::Instance().SetIcon(EnginePaths::GetResourcePath() + "/Insight/default.png");
			Graphics::Window::Instance().Show();

			EditorWindowManager::Instance().RegisterWindows();
			m_menuBar.Initialise(&m_editorWindowManager);
		}

		void Editor::OnUpdate()
		{
			if (!Runtime::ProjectSystem::Instance().IsProjectOpen())
			{
				m_editorWindowManager.AddWindow(ProjectWindow::WINDOW_NAME);
				const ProjectWindow* projectWindow = static_cast<const ProjectWindow*>(m_editorWindowManager.GetActiveWindow(ProjectWindow::WINDOW_NAME));
				RemoveConst(projectWindow)->SetFullscreen(true);
			}
			else
			{
				m_menuBar.Draw();
			}

			EditorWindowManager::Instance().Update();
		}

		void Editor::OnDestroy()
		{
			EditorWindowManager::Instance().Destroy();
		}
	}
}

Insight::App::Engine* CreateApplication()
{
	return NewTracked(Insight::Editor::Editor);
}
