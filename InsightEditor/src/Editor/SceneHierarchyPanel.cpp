#include "ispch.h"
#include "Editor/SceneHierarchyPanel.h"
#include "Insight/Scene/Scene.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/Component.h"
#include "Insight/Input/Input.h"
#include "Insight/Event/EventManager.h"
#include "Editor/EditorDrawer.h"

#include "Editor/RTTIToImGUI.h"
#include "Insight/Component/CameraComponent.h"
#include "Insight/Component/MeshComponent.h"
#include "imgui.h"


	namespace Editor
	{
		SceneHierarchyPanel::SceneHierarchyPanel(SharedPtr<Module::EditorModule> editorModule)
			: EditorWindow(editorModule)
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
						Entity::Create();
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
					auto allowRemovableProb = IS_GET_PROPERTY((*componentsIT).get(), "Allow_Removable");

					ImGui::Separator();
					ImGui::Text((*componentsIT)->GetType().GetTypeName().c_str());
					if (allowRemovableProb.IsValid() && allowRemovableProb.GetPropertyValue<bool>())
					{
						ImGui::SameLine(ImGui::GetWindowWidth() - 30);
						if (ImGui::Button("X"))
						{
							// TODO: Remove component
							spEntity->RemoveComponent((*componentsIT)->GetUUID());
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
						if (ImGui::Button("Camera Component")) { spEntity->AddComponent<CameraComponent>(); }
						else if (ImGui::Button("Mesh Component")) { spEntity->AddComponent<MeshComponent>(); }
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