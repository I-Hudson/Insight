#include "Editor/Editor.h"

#include "EditorModule.h"

#include "Runtime/EntryPoint.h"
#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"

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

			std::string windowTitle = "Insight Editor (";
			windowTitle += Graphics::GraphicsAPIToString(Graphics::RenderContext::Instance().GetGraphicsAPI());
			windowTitle += ")";

			Graphics::Window::Instance().SetTite(windowTitle);
			Graphics::Window::Instance().SetIcon("./Resources/Insight/default.png");
			Graphics::Window::Instance().Show();

			EditorWindowManager::Instance().RegisterWindows();
			m_menuBar.Initialise(&m_editorWindowManager);

			EditorWindowManager::Instance().AddWindow("ResourceWindow");
			EditorWindowManager::Instance().AddWindow("EntitiesWindow");
			EditorWindowManager::Instance().AddWindow("InputWindow");
		}

		void Editor::OnUpdate()
		{
			m_menuBar.Draw();
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
