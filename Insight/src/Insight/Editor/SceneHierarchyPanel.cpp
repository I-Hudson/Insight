#include "ispch.h"
#include "Insight/Editor/SceneHierarchyPanel.h"
#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/Component.h"
#include "Insight/Input/Input.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Editor/EditorDrawer.h"

#include "Insight/Editor/RTTIToImGUI.h"
#include "imgui.h"

namespace Insight
{
	namespace Editor
	{
		SceneHierarchyPanel::SceneHierarchyPanel(SharedPtr<Module::EditorModule> editorModule)
			: EditorPanel(editorModule)
		{
			SET_PANEL_NAME(SceneHierarchyPanel);
		}

		SceneHierarchyPanel::~SceneHierarchyPanel()
		{
		}

		void SceneHierarchyPanel::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();
#ifdef IMGUI_ENABLED

			ImGui::Begin("Scene Hierarchy Panel");
			bool titleBarHovered = ImGui::IsItemHovered();

			ImGui::TreePush("Entites");
			bool newEntitySelected = false;
			auto entites = Scene::ActiveScene()->GetEntites();

			for (auto entitesIT = entites.begin(); entitesIT != entites.end(); ++entitesIT)
			{
				if ((*entitesIT)->GetParent().expired())
				{
					if (ImGui::TreeNodeEx((*entitesIT).get(), GetTreeNodeFlags(*entitesIT), (*entitesIT)->GetID().c_str()))
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

		void SceneHierarchyPanel::DrawEntityTreeView(WeakPtr<Entity> entity, bool& newEntitySelected)
		{
			if (auto spEntity = entity.lock())
			{
#ifdef IMGUI_ENABLED
				for (unsigned int i = 0; i < spEntity->GetChildCount(); ++i)
				{
					SharedPtr<Entity> currentEntity = spEntity->GetChild(i);
					if (ImGui::TreeNodeEx(currentEntity.get(), GetTreeNodeFlags(currentEntity), currentEntity->GetID().c_str()))
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
		}

		int SceneHierarchyPanel::GetTreeNodeFlags(WeakPtr<Entity> entity)
		{
			if (auto spEntity = entity.lock())
			{
#ifdef IMGUI_ENABLED
				ImGuiTreeNodeFlags flags = /*ImGuiTreeNodeFlags_Framed |*/ ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (spEntity->GetChildCount() == 0)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}

				if (m_selectedEntity == spEntity)
				{
					flags |= ImGuiTreeNodeFlags_Selected;
				}

				return flags;
#endif
			}
			return 0;
		}

		void SceneHierarchyPanel::DrawCompoentPanel(WeakPtr<Entity> entity)
		{
			//IS_TODO("Remove this and place in it's own class!!!");

			IS_PROFILE_FUNCTION();
#ifdef IMGUI_ENABLED
			ImGui::Begin("Components Panel");

			if (auto spEntity = entity.lock())
			{
				ImGui::InputText(":Name", &spEntity->GetID());
				ImGui::Checkbox(":Active", &spEntity->IsActive());
				ImGui::Checkbox(":Debug", &spEntity->ShowDebugInfo());

				if (spEntity->ShowDebugInfo())
				{
					ImGui::LabelText(":UUID", spEntity->GetUUID().c_str());
				}

				ImGui::NewLine();

				auto components = spEntity->GetAllComponents();

				for (auto componentsIT = components.begin(); componentsIT != components.end(); ++componentsIT)
				{
					IS_PROFILE_SCOPE("Draw Properties");
					auto properties = IS_GET_ALL_PROPERTIES((*componentsIT).get(), ShowInEditor);

					ImGui::Separator();
					ImGui::Text((*componentsIT)->GetTypeName().c_str());

					if (!Editor::EditorDrawerRegistry::Instance()->CallEditorDrawer((*componentsIT)->GetTypeName().c_str(), *(*componentsIT).get()))
					{
						for (auto propertyIT = properties.begin(); propertyIT != properties.end(); ++propertyIT)
						{
							if ((*propertyIT)->IsValid())
							{
								RTTIToImGUI_Input((*propertyIT)->GetObjectPtr(), (*propertyIT)->GetType(), (*propertyIT)->GetPropertyName(), (*propertyIT)->GetPropertyFlags());
								ImGui::Spacing();
							}
						}
					}
				}
				ImGui::Separator();
			}

			ImGui::End();
#endif
		}
	}
}