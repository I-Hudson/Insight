#include "Editor.h"

#include "Runtime/EntryPoint.h"
#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"

#include "Graphics/Window.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		void Editor::OnInit()
		{
			Graphics::Window::Instance().SetIcon("./Resources/Insight/default.png");
			Graphics::Window::Instance().Show();
			ImGui::SetCurrentContext(Core::ImGuiSystem::GetCurrentContext());

			EditorWindowManager::Instance().RegisterWindows();
			EditorWindowManager::Instance().AddWindow("ResourceWindow");
		}

		void Editor::OnUpdate()
		{
			if (ImGui::Button("Resource window"))
			{
				EditorWindowManager::Instance().AddWindow("ResourceWindow");
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
