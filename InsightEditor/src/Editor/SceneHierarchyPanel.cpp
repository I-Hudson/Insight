#include "ispch.h"
#include "Editor/SceneHierarchyPanel.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Entitiy/Entity.h"
#include "Engine/Component/Component.h"
#include "Engine/Input/Input.h"
#include "Engine/Event/EventManager.h"
#include "Editor/EditorDrawer.h"

#include "Editor/RTTIToImGUI.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "imgui.h"


	namespace Editor
	{
		SceneHierarchyPanel::SceneHierarchyPanel(const Module::EditorModule* editorModule)
			: EditorWindow(editorModule)
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
#ifdef IMGUI_ENABLED

			ImGui::Begin("Scene Hierarchy Panel");
			bool titleBarHovered = ImGui::IsItemHovered();

			ImGui::TreePush("Entites");
			bool newEntitySelected = false;
			auto entites = Scene::ActiveScene()->GetEntites();

			for (auto entitesIT = entites.begin(); entitesIT != entites.end(); ++entitesIT)
			{
				if ((*entitesIT)->GetParent())
				{
					if (ImGui::TreeNodeEx((*entitesIT), GetTreeNodeFlags(*entitesIT), (*entitesIT)->GetID().c_str()))
					{
						DrawEntityTreeView((*entitesIT), newEntitySelected);
						ImGui::TreePop();
					}

					if (ImGui::IsItemHovered() && Input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !newEntitySelected)
					{
						m_selectedEntity = *entitesIT;
						newEntitySelected = true;
					}
				}
			}
			ImGui::TreePop();

			if (!titleBarHovered && ImGui::IsWindowHovered() && Input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !newEntitySelected)
			{
				m_selectedEntity = nullptr;
			}

			if (IsMouseInBounds() && Input::IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
			{
				if (!m_openContextPopup)
				{
					m_openContextPopup = true;
				}
			}

			if (m_openContextPopup)
			{
				ImGui::OpenPopup("Context Menu");
				if (ImGui::BeginPopup("Context Menu"))
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));

					if (ImGui::Button("Create Entity"))
					{
						Entity::New();
					}

					ImGui::PopStyleColor();
					ImGui::EndPopup();
				}

				if ((!IsMouseInBounds() && (Input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || Input::IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))) || Input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
				{
					m_openContextPopup = false;
				}
			}
			

			ImGui::End();

			DrawCompoentPanel(m_selectedEntity);
#endif
		}

		void SceneHierarchyPanel::DrawEntityTreeView(Entity* entity, bool& newEntitySelected)
		{
			if (entity)
			{
#ifdef IMGUI_ENABLED
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
		}

		int SceneHierarchyPanel::GetTreeNodeFlags(Entity* entity)
		{
			if (entity)
			{
#ifdef IMGUI_ENABLED
				ImGuiTreeNodeFlags flags = /*ImGuiTreeNodeFlags_Framed |*/ ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
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
			}
			return 0;
		}

		void SceneHierarchyPanel::DrawCompoentPanel(Entity* entity)
		{
			//IS_TODO("Remove this and place in it's own class!!!");

			IS_PROFILE_FUNCTION();
#ifdef IMGUI_ENABLED
			ImGui::Begin("Components Panel");

			if (entity)
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

				for (auto componentsIT = components.begin(); componentsIT != components.end(); ++componentsIT)
				{
					IS_PROFILE_SCOPE("Draw Properties");
					auto properties = IS_GET_ALL_PROPERTIES((*componentsIT), ShowInEditor);
					auto allowRemovableProb = IS_GET_PROPERTY((*componentsIT), "Allow_Removable");

					ImGui::Separator();
					ImGui::Text((*componentsIT)->GetType().GetTypeName().c_str());
					if (allowRemovableProb.IsValid() && allowRemovableProb.GetPropertyValue<bool>())
					{
						ImGui::SameLine(ImGui::GetWindowWidth() - 30);
						if (ImGui::Button("X"))
						{
							// TODO: Remove component
							entity->RemoveComponent((*componentsIT)->GetUUID());
							continue;
						}
					}

					if (!Editor::EditorDrawerRegistry::CallEditorDrawer((*componentsIT)->GetType(), *(*componentsIT)))
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

				const char* addComponentButtonText = "+";
				//ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - ImGui::CalcTextSize(addComponentButtonText).x) * 0.5f, ImGui::GetCursorPosY()));

				if (m_openAddComponentPopup)
				{
					ImGui::OpenPopup("Add Component Menu");
					if (ImGui::BeginPopup("Add Component Menu"))
					{
						if (ImGui::Button("Camera Component")) { entity->AddComponent<CameraComponent>(); }
						else if (ImGui::Button("Mesh Component")) { entity->AddComponent<MeshComponent>(); }
						ImGui::EndPopup();
					}

					if (Input::IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
					{
						m_openAddComponentPopup = false;
					}
				}

				if (ImGui::Button(addComponentButtonText, ImVec2(ImGui::GetWindowWidth(), 35.f)))
				{
					// TODO: Add component
					m_openAddComponentPopup = true;
				}
			}
			ImGui::End();
#endif
		}
	}