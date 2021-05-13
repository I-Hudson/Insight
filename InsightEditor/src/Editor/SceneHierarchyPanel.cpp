#include "ispch.h"
#include "Editor/SceneHierarchyPanel.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Component/Component.h"
#include "Engine/Input/Input.h"
#include "Engine/Event/EventManager.h"
#include "Editor/EditorDrawer.h"

#include "Editor/RTTIToImGUI.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/MeshComponent.h"
#include "ReflectStructs.h"
#include "imgui.h"


	namespace Editor
	{
		SceneHierarchyPanel::SceneHierarchyPanel(const Module::EditorModule* editorModule)
			: EditorWindow(editorModule)
			, m_selectedEntity(nullptr, -1)
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
				if (!(*entitesIT).GetParent().IsValid())
				{
					std::string entityId =std::to_string((*entitesIT).GetEntityID());
					if (ImGui::TreeNodeEx(entityId.c_str(), GetTreeNodeFlags(*entitesIT), (*entitesIT).GetName().c_str()))
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
				m_selectedEntity = Entity(nullptr, -1);
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
						Scene::ActiveScene()->CreateEntity();
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

		void SceneHierarchyPanel::DrawEntityTreeView(Entity& entity, bool& newEntitySelected)
		{
			if (entity.IsValid())
			{
#ifdef IMGUI_ENABLED
				for (unsigned int i = 0; i < entity.GetChildCount(); ++i)
				{
					Entity childEntity = entity.GetChild(i);
					std::string entityId = std::to_string(childEntity.GetEntityID());
					if (ImGui::TreeNodeEx(entityId.c_str(), GetTreeNodeFlags(childEntity), childEntity.GetName().c_str()))
					{
						DrawEntityTreeView(childEntity, newEntitySelected);
						ImGui::TreePop();
					}

					if (ImGui::IsItemClicked() && !newEntitySelected)
					{
						m_selectedEntity = childEntity;
						newEntitySelected = true;
					}
				}
#endif
			}
		}

		int SceneHierarchyPanel::GetTreeNodeFlags(Entity& entity)
		{
			if (entity.IsValid())
			{
#ifdef IMGUI_ENABLED
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (entity.GetChildCount() == 0)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}

				if (m_selectedEntity.GetEntityID() == entity.GetEntityID())
				{
					flags |= ImGuiTreeNodeFlags_Selected;
				}

				return flags;
#endif
			}
			return 0;
		}

		void SceneHierarchyPanel::DrawCompoentPanel(Entity& entity)
		{
			//IS_TODO("Remove this and place in it's own class!!!");

			IS_PROFILE_FUNCTION();
#if 1
			ImGui::Begin("Components Panel");

			if (entity.IsValid())
			{
				std::string entityName = entity.GetName();
				ImGui::InputText(":Name", &entityName);
				entity.SetName(entityName);

				ImGui::Checkbox(":Active", &entity.GetEntiyData().IsActive);
				//ImGui::Checkbox(":Debug", &entity.ShowDebugInfo());

				if (entity.GetEntiyData().GetMember("ShowDebug").ConvertToType<bool>())
				{
					std::string entityID = std::to_string(entity.GetEntityID());
					ImGui::LabelText(":EntityID", entityID.c_str());
				}
				ImGui::NewLine();

				std::vector<Component> removedComponents;
				for (u32 i = 0; i < entity.GetComponentCount(); ++i)
				{
					auto& component = entity.GetComponent(i);
					IS_PROFILE_SCOPE("Draw Properties");
					auto properties = component.GetMembers({ "ShowInEditor" });
					auto& componentData = component.GetComponentData();
					auto dataProperties = componentData.GetMember("AllowRemovable");

					ImGui::Separator();
					ImGui::Text(component.GetType().GetTypeName().c_str());
					if (dataProperties.IsValid() && *dataProperties.ConvertToType<bool>())
					{
						ImGui::SameLine(ImGui::GetWindowWidth() - 150);
						if (ImGui::Button("X"))
						{
							removedComponents.push_back(component);
							continue;
						}
					}

					if (!Editor::EditorDrawerRegistry::CallEditorDrawer(component.GetType(), component))
					{
						for (auto propertyIT = properties.begin(); propertyIT != properties.end(); ++propertyIT)
						{
							if ((*propertyIT).IsValid())
							{
								RTTIToImGUI_Input((*propertyIT).GetRawPointer(), (*propertyIT).GetTypeName(), (*propertyIT).GetName(), ShowInEditor);
								ImGui::Spacing();
							}
						}
					}
				}
				for (auto& component : removedComponents)
				{
					entity.RemoveComponent(component);
				}

				ImGui::Separator();

				const char* addComponentButtonText = "+";
				//ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - ImGui::CalcTextSize(addComponentButtonText).x) * 0.5f, ImGui::GetCursorPosY()));

				if (m_openAddComponentPopup)
				{
					ImGui::OpenPopup("Add Component Menu");
					if (ImGui::BeginPopup("Add Component Menu"))
					{
						if (ImGui::Button("Camera Component")) { entity.AddComponent<CameraComponent>(); }
						else if (ImGui::Button("Mesh Component")) { entity.AddComponent<MeshComponent>(); }
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