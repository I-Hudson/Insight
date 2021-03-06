#include "Editor/TopBarMenu.h"
#include "ImGuiFileBrowser.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Entity/Entity.h"

namespace Insight::Editor
{
	TopBarMenu::TopBarMenu(const Module::EditorModule* editorModule)
		: EditorWindow(editorModule)
	{
		SET_PANEL_NAME(TopBarMenu);
	}

	TopBarMenu::~TopBarMenu()
	{
	}

	void TopBarMenu::Update(const float& deltaTime)
	{
		bool save = false, open = false;
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene")) { Scene::ActiveScene()->Unload(); }
				else if (ImGui::MenuItem("Save Scene")) { save = true; }
				else if (ImGui::MenuItem("Load Scene")) { open = true; }
#ifdef IS_DEBUG
				else if (ImGui::MenuItem("Create Debug Large Scene"))
				{
					int r = rand() % 100 + 50;
					for (size_t i = 0; i < r; ++i)
					{
						Scene::ActiveScene()->CreateEntity(std::to_string(i));
					}
				}
#endif
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::Button("Start Scene")) { Scene::ActiveScene()->Play(); }
				if (ImGui::Button("Stop Scene")) { Scene::ActiveScene()->End(); }

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (save)
		{
			ImGui::OpenPopup("Save Scene");
		}
		else if (open)
		{
			ImGui::OpenPopup("Open Scene");
		}

		if (m_fileDialog.showFileDialog("Save Scene", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(700, 310), ".xml"))
		{
			Scene::ActiveScene()->Serialize(m_fileDialog.selected_path);

		}
		else if (m_fileDialog.showFileDialog("Open Scene", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".xml"))
		{
			Scene::ActiveScene()->Deserialize(m_fileDialog.selected_path);
		}
	}
}