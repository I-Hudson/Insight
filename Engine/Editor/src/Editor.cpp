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

			ImGui::SetCurrentContext(GetSystemRegistry().GetSystem<Core::ImGuiSystem>()->GetCurrentContext());
			ImGuiMemAllocFunc allocFunc;
			ImGuiMemFreeFunc freeFunc;
			void* pUsedData;
			GetSystemRegistry().GetSystem<Core::ImGuiSystem>()->GetAllocatorFunctions(allocFunc, freeFunc, pUsedData);
			ImGui::SetAllocatorFunctions(allocFunc, freeFunc, pUsedData);

			EditorWindowManager::Instance().RegisterWindows();
			EditorWindowManager::Instance().AddWindow("ResourceWindow");
			EditorWindowManager::Instance().AddWindow("EntitiesWindow");
			EditorWindowManager::Instance().AddWindow("InputWindow");
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
