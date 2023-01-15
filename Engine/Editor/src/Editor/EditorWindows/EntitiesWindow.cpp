#include "Editor/EditorWindows/EntitiesWindow.h"
#include "World/WorldSystem.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include <imgui.h>

namespace Insight
{
	namespace Editor
	{
		EntitiesWindow::EntitiesWindow()
			: IEditorWindow()
		{ }

		EntitiesWindow::EntitiesWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		EntitiesWindow::EntitiesWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		EntitiesWindow::~EntitiesWindow()
		{ }

		void EntitiesWindow::OnDraw()
		{
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
							DrawSingleEntity(entities.at(i).Get());
						}
					}
				}
			}
		}

		std::unordered_set<Core::GUID> const& EntitiesWindow::GetSelectedEntities() const
		{
			return m_selectedEntities;
		}

		void EntitiesWindow::DrawSingleEntity(ECS::Entity* entity)
		{
			bool isLeaf = entity->GetChildCount() == 0;
			bool isSelected = m_selectedEntities.find(entity->GetGUID()) != m_selectedEntities.end();

			ImGuiTreeNodeFlags treeNodeFlags =
				ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
				| (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

			bool isEnabled = entity->IsEnabled();
			if (ImGui::Checkbox(std::string("##" + entity->GetGUID().ToString()).c_str(), &isEnabled))
			{
				entity->SetEnabled(isEnabled);
			}
			ImGui::SameLine();
			if (ImGui::TreeNodeEx(entity->GetName().c_str(), treeNodeFlags))
			{
				IsEntitySelected(entity, isSelected);

				for (u32 i = 0; i < entity->GetChildCount(); ++i)
				{
					DrawSingleEntity(entity->GetChild(i).Get());
				}
				ImGui::TreePop();
			}
			else
			{
				IsEntitySelected(entity, isSelected);
			}
		}

		void EntitiesWindow::IsEntitySelected(ECS::Entity* entity, bool isSelected)
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