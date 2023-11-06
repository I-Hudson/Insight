#include "Editor/EditorWindows/WorldEntitiesWindow.h"
#include "World/WorldSystem.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Algorithm/Vector.h"

#include "Event/EventSystem.h"
#include "Runtime/RuntimeEvents.h"

#include "Core/Profiler.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		WorldEntitiesWindow::WorldEntitiesWindow()
			: IEditorWindow()
		{ }

		WorldEntitiesWindow::WorldEntitiesWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		WorldEntitiesWindow::WorldEntitiesWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		WorldEntitiesWindow::~WorldEntitiesWindow()
		{ }

		void WorldEntitiesWindow::Initialise()
		{
			Core::EventSystem::Instance().AddEventListener(this, Core::EventType::WorldDestroy, [this](const Core::Event& e)
			{
				const WorldDestroyEvent& worldDestroyEvent = static_cast<const WorldDestroyEvent&>(e);

				std::vector<Core::GUID> removeEntities;
				for (const Core::GUID& guid : m_selectedEntities)
				{
					if (worldDestroyEvent.World->GetEntityByGUID(guid))
					{
						removeEntities.push_back(guid);
					}
				}

				for (const Core::GUID& guid : removeEntities)
				{
					m_selectedEntities.erase(guid);
				}
			});
		}

		void WorldEntitiesWindow::Shutdown()
		{
			Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::WorldDestroy);
		}

		void WorldEntitiesWindow::OnDraw()
		{
			IS_PROFILE_FUNCTION();

			std::vector<TObjectPtr<Runtime::World>> worlds = Runtime::WorldSystem::Instance().GetAllWorlds();
			for (size_t i = 0; i < worlds.size(); ++i)
			{
				if (auto world = worlds.at(i))
				{
					if (world->GetWorldName().find("Editor") != std::string::npos)
					{
						continue;
					}

					if (ImGui::CollapsingHeader(world->GetWorldName().data()))
					{
						auto entities = world->GetAllEntities();
						for (size_t i = 0; i < entities.size(); ++i)
						{
							DrawSingleEntity(entities.at(i).Get(), static_cast<u32>(i));
						}
					}
				}
			}

			TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
			if (worlds.size() > 0)
			{
				const Input::InputDevice_KeyboardMouse* inputDevice = Input::InputSystem::Instance().GetKeyboardMouseDevice();
				if (inputDevice->WasReleased(Input::MouseButtons::Right)
					&& IsCursorWithinWindow())
				{
					m_showContextMenu = true;
				}
				
				if (m_showContextMenu == true)
				{
					ImGui::OpenPopup("##WEW_CONTEXT_MENU");
				}

				if (ImGui::BeginPopup("##WEW_CONTEXT_MENU"))
				{
					if (ImGui::MenuItem("Add Entity"))
					{
						activeWorld->AddEntity();
					}
					
					ImGui::BeginDisabled(m_selectedEntities.size() == 0);
					if (ImGui::MenuItem("Remove Entities"))
					{
						for (const Core::GUID& entityGuid : m_selectedEntities)
						{
							Ptr<ECS::Entity> entityToRemove = activeWorld->GetEntityByGUID(entityGuid);
							if (entityToRemove)
							{
								activeWorld->RemoveEntity(entityToRemove);
							}
						}
						m_selectedEntities.clear();
					}
					ImGui::EndDisabled();

					if (inputDevice->WasReleased(Input::MouseButtons::Left)
						&& !ImGui::IsItemHovered())
					{
						m_showContextMenu = false;
					}

					ImGui::EndPopup();
				}


				if (inputDevice->WasReleased(Input::MouseButtons::Left)
					&& !IsCursorWithinWindow())
				{
					m_showContextMenu = false;
				}
			}
		}

		std::unordered_set<Core::GUID> const& WorldEntitiesWindow::GetSelectedEntities() const
		{
			return m_selectedEntities;
		}

		void WorldEntitiesWindow::DrawSingleEntity(ECS::Entity* entity, u32 entityIndex)
		{
			if (!entity)
			{
				return;
			}

			bool isLeaf = entity->GetChildCount() == 0;
			bool isSelected = m_selectedEntities.find(entity->GetGUID()) != m_selectedEntities.end();

			ImGuiTreeNodeFlags treeNodeFlags =
				ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
				| (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

			bool isEnabled = entity->IsEnabled();
			if (ImGui::Checkbox(std::string("##EntityEnabled" + entity->GetGUID().ToString()).c_str(), &isEnabled))
			{
				entity->SetEnabled(isEnabled);
			}
			ImGui::SameLine();

			ImGui::PushID(entityIndex);

			std::string entityName = entity->GetName();
			if (ImGui::TreeNodeEx(entityName.c_str(), treeNodeFlags))
			{
				IsEntitySelected(entity, isSelected);

				for (u32 i = 0; i < entity->GetChildCount(); ++i)
				{
					DrawSingleEntity(entity->GetChild(i).Get(), entityIndex + i);
				}
				ImGui::TreePop();
			}
			else
			{
				IsEntitySelected(entity, isSelected);
			}

			ImGui::PopID();
		}

		void WorldEntitiesWindow::IsEntitySelected(ECS::Entity* entity, bool isSelected)
		{
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				Input::InputDevice_KeyboardMouse* inputDevice = Input::InputSystem::Instance().GetKeyboardMouseDevice();
				if (inputDevice->WasHeld(Input::KeyboardButtons::Key_LCtrl))
				{
					m_selectedEntities.insert(entity->GetGUID());
					for (u32 i = 0; i < entity->GetChildCount(); ++i)
					{
						//m_selectedEntities.insert(entity->GetChild(i)->GetGUID());
					}
				}
				else
				{
					m_selectedEntities.clear();
					if (!isSelected)
					{
						m_selectedEntities.insert(entity->GetGUID());
						for (u32 i = 0; i < entity->GetChildCount(); ++i)
						{
							//m_selectedEntities.insert(entity->GetChild(i)->GetGUID());
						}
					}
				}
			}
		}
	}
}