#include "Editor/Editor.h"

#include "EditorModule.h"
#include "Editor/EditorWindows/ProjectWindow.h"
#include "Editor/EditorWindows/GameViewWindow.h"
#include "Editor/EditorWindows/WorldViewWindow.h"

#include "Runtime/EntryPoint.h"
#include "Resource/ResourceManager.h"

#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"

#include "Serialisation/Archive.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		IS_SERIALISABLE_CPP(Editor);

		void Editor::OnInit()
		{
			IS_PROFILE_FUNCTION();

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

			m_editorWindowManager.RegisterWindows();
			m_editorWindowManager.AddWindow(WorldViewWindow::WINDOW_NAME);

			m_menuBar.Initialise(&m_editorWindowManager);

			m_gameRenderpass = New<Graphics::Renderpass>();
			m_gameRenderpass->Create();

			App::Engine::Instance().GetSystemRegistry().RegisterSystem(&m_projectCodeSystem);
			m_projectCodeSystem.Initialise();

			glm::vec4 vec4A;
			glm::vec4 vec4B;

			auto vecC = vec4A * vec4B;

			glm::mat4 mat4A;
			glm::mat4 mat4B;
			auto mat4C = mat4A * mat4B;

			Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Read);
			if (!editorSettings.IsEmpty())
			{
				EditorSettingsSerialiser serialiser(true);
				serialiser.Deserialise(editorSettings.GetData());
				Deserialise(&serialiser);
			}
		}

		void Editor::OnUpdate()
		{
			IS_PROFILE_FUNCTION();

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

		void Editor::OnRender()
		{
			IS_PROFILE_FUNCTION();

			const bool gameViewWindowActive = m_editorWindowManager.GetActiveWindow(GameViewWindow::WINDOW_NAME) != nullptr;
			m_gameRenderpass->FrameSetup();
			m_gameRenderpass->RenderMainPasses(gameViewWindowActive);
			m_gameRenderpass->RenderSwapchain(false);
			m_gameRenderpass->RenderPostprocessing();
		}

		void Editor::OnDestroy()
		{
			IS_PROFILE_FUNCTION();

			EditorSettingsSerialiser serialiser(false);
			Serialise(&serialiser);

			Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Write);
			editorSettings.Write(serialiser.GetSerialisedData());
			editorSettings.Close();

			m_gameRenderpass->Destroy();
			Delete(m_gameRenderpass);

			EditorWindowManager::Instance().Destroy();

			m_projectCodeSystem.Shutdown();
			App::Engine::Instance().GetSystemRegistry().UnregisterSystem(&m_projectCodeSystem);

			Runtime::ResourceManager::SaveDatabase();
		}
	}
}

Insight::App::Engine* CreateApplication()
{
	return New<Insight::Editor::Editor, Insight::Core::MemoryAllocCategory::Editor>();
}
