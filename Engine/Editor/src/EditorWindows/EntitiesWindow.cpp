#include "EditorWindows/EntitiesWindow.h"
#include "Scene/SceneManager.h"
#include "Input/InputManager.h"

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
			std::vector<WPtr<App::Scene>> scenes = App::SceneManager::Instance().GetAllScenes();
			for (size_t i = 0; i < scenes.size(); ++i)
			{
				if (auto scene = scenes.at(i).Lock())
				{
					if (scene->GetSceneName().find("Editor") != std::string::npos)
					{
						continue;
					}

					if(ImGui::CollapsingHeader(scene->GetSceneName().data()))
					{
						auto entities = scene->GetAllEntities();
						for (size_t i = 0; i < entities.size(); ++i)
						{
							DrawSingleEntity(entities.at(i).Get());
						}
					}
				}
			}
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
			
			if (ImGui::TreeNodeEx(entity->GetName().c_str(), treeNodeFlags))
			{
				if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				{
					if (Input::InputManager::IsKeyPressed(IS_KEY_LEFT_CONTROL))
					{
						m_selectedEntities.insert(entity->GetGUID());
						for (u32 i = 0; i < entity->GetChildCount(); ++i)
						{
							m_selectedEntities.insert(entity->GetChild(i)->GetGUID());
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
								m_selectedEntities.insert(entity->GetChild(i)->GetGUID());
							}
						}
					}
				}

				for (u32 i = 0; i < entity->GetChildCount(); ++i)
				{
					DrawSingleEntity(entity->GetChild(i).Get());
				}
				ImGui::TreePop();
			}
		}
	}
}