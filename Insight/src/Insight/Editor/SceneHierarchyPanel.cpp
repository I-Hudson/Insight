#include "ispch.h"
#include "Insight/Editor/SceneHierarchyPanel.h"
#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/Component.h"
#include "Insight/Input/Input.h"
#include "Insight/Event/EventManager.h"

#include "Insight/Editor/RTTIToImGUI.h"
#include "imgui.h"

namespace Insight
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Module::EditorModule* editorModule)
		: EditorPanel(editorModule)
		, m_selectedEntity(nullptr)
	{
		SET_PANEL_NAME(SceneHierarchyPanel);
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();
#ifdef IMGUI_ENABLE

		ImGui::Begin("Scene Hierarchy Panel");
		bool titleBarHovered = ImGui::IsItemHovered();

		ImGui::TreePush("Entites");
		bool newEntitySelected = false;
		auto entites = Scene::ActiveScene()->GetEntites();

		for (auto entitesIT = entites.begin(); entitesIT != entites.end(); ++entitesIT)
		{
			if ((*entitesIT)->GetParent() == nullptr)
			{
				if (ImGui::TreeNodeEx((*entitesIT), GetTreeNodeFlags(*entitesIT), (*entitesIT)->GetID().c_str()))
				{
					DrawEntityTreeView((*entitesIT), newEntitySelected);
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked() && !newEntitySelected)
				{
					m_selectedEntity = *entitesIT;
					newEntitySelected = true;
				}
			}
		}
		ImGui::TreePop();

		if (!titleBarHovered && ImGui::IsWindowHovered() && Input::MouseButtonDown(MOUSE_BUTTON_LEFT) && !newEntitySelected)
		{
			m_selectedEntity = nullptr;
		}

		ImGui::End();

		DrawCompoentPanel(m_selectedEntity);
#endif
	}

	void SceneHierarchyPanel::DrawEntityTreeView(Entity* entity, bool& newEntitySelected)
	{
#ifdef IMGUI_ENABLE
		for (unsigned int i = 0; i < entity->GetChildCount(); ++i)
		{
			Entity* currentEntity = entity->GetChild(i);
			if (ImGui::TreeNodeEx(currentEntity, GetTreeNodeFlags(currentEntity), currentEntity->GetID().c_str()))
			{
				DrawEntityTreeView(currentEntity, newEntitySelected);
				ImGui::TreePop();
			}

			if (ImGui::IsItemClicked() && !newEntitySelected)
			{
				m_selectedEntity = currentEntity;
				newEntitySelected = true;
			}
		}
#endif
	}

	int SceneHierarchyPanel::GetTreeNodeFlags(Entity* entity)
	{
#ifdef IMGUI_ENABLE
		ImGuiTreeNodeFlags flags = /*ImGuiTreeNodeFlags_Framed |*/ ImGuiTreeNodeFlags_OpenOnArrow;
		if (entity->GetChildCount() == 0)
		{
			flags |= ImGuiTreeNodeFlags_Leaf;
		}

		if (m_selectedEntity == entity)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		return flags;
#endif
		return 0;
	}

	void SceneHierarchyPanel::DrawCompoentPanel(Entity* entity)
	{
		//IS_TODO("Remove this and place in it's own class!!!");

		IS_PROFILE_FUNCTION();
#ifdef IMGUI_ENABLE
		ImGui::Begin("Components Panel");

		if (entity != nullptr)
		{
			ImGui::InputText(":Name", &entity->GetID());
			ImGui::Checkbox(":Active", &entity->IsActive());
			ImGui::Checkbox(":Debug", &entity->ShowDebugInfo());

			if (entity->ShowDebugInfo())
			{
				ImGui::LabelText(":UUID", entity->GetUUID().c_str());
			}

			ImGui::NewLine();

			auto components = entity->GetAllComponents();

			for (auto componentsIT = components->begin(); componentsIT != components->end(); ++componentsIT)
			{
				IS_PROFILE_SCOPE("Draw Properties");
				auto properties = IS_GET_ALL_PROPERTIES(*componentsIT, RTTIPropertyEditorFlags_ShowInEditor);

				ImGui::Separator();

				for (auto propertyIT = properties.begin(); propertyIT != properties.end(); ++propertyIT)
				{
					RTTIToImGUI_Input((*propertyIT)->GetObjectPtr(), (*propertyIT)->GetType(), (*propertyIT)->GetPropertyName(), (*propertyIT)->GetPropertyEditorFlags());
					ImGui::Spacing();
				}
			}
			ImGui::Separator();
		}

		ImGui::End();
#endif
	}
}