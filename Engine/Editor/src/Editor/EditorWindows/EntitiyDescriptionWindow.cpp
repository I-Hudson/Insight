#include "Editor/EditorWindows/EntitiyDescriptionWindow.h"
#include "Editor/EditorWindows/EntitiesWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "World/WorldSystem.h"

#include "Core/Memory.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        EntitiyDescriptionWindow::EntitiyDescriptionWindow()
            : IEditorWindow()
        {}

        EntitiyDescriptionWindow::EntitiyDescriptionWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        EntitiyDescriptionWindow::EntitiyDescriptionWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        EntitiyDescriptionWindow::~EntitiyDescriptionWindow()
        { }

        void EntitiyDescriptionWindow::OnDraw()
        {
            if (EditorWindowManager::Instance().IsWindowVisable(EntitiesWindow::WINDOW_NAME))
            {
                EntitiesWindow const* entitiesWindow = static_cast<EntitiesWindow const*>(EditorWindowManager::Instance().GetActiveWindow(EntitiesWindow::WINDOW_NAME));
                std::unordered_set<Core::GUID> const& selectedEntities = entitiesWindow->GetSelectedEntities();
                if (selectedEntities.size() == 0)
                {
                    return;
                }
                else
                {
                    Core::GUID selctedEntityGUID = *selectedEntities.begin();     
                    ECS::Entity* selectedEntity = FindEntity(selctedEntityGUID);
                    DrawEntity(selectedEntity);
                }
            }
        }

        ECS::Entity* EntitiyDescriptionWindow::FindEntity(Core::GUID guid)
        {
            ECS::Entity* selectedEntity = nullptr;
            std::vector<TObjectPtr<Runtime::World>> worlds = Runtime::WorldSystem::Instance().GetAllWorlds();
            for (size_t i = 0; i < worlds.size(); ++i)
            {
                if (auto world = worlds.at(i))
                {
                    selectedEntity = world->GetEntityByGUID(guid);
                    if (selectedEntity)
                    {
                        break;
                    }
                }
            }
            return selectedEntity;
        }

        void EntitiyDescriptionWindow::DrawEntity(ECS::Entity* entity)
        {
            ImGui::Text("Entity");
            ImGui::Separator();

            ImGui::Text("Entity GUID: %s", entity->GetGUID().ToString().c_str());
            ImGui::Text("Entity Name: %s", entity->GetName().c_str());
            bool isEnabled = entity->IsEnabled();
            if (ImGui::Checkbox("Entity Enabled: %s", &isEnabled)) { entity->SetEnabled(isEnabled); }

            for (size_t i = 0; i < entity->GetComponentCount(); ++i)
            {
                ImGui::Spacing();
                ImGui::Spacing();
                DrawComponent(entity->GetComponentByIndex(i));
            }
        }

        void EntitiyDescriptionWindow::DrawComponent(ECS::Component* component)
        {
            ImGui::Text("%s", component->GetTypeName());
            ImGui::Separator();
        }
    }
}