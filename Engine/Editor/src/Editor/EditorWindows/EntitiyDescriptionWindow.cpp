#include "Editor/EditorWindows/EntitiyDescriptionWindow.h"
#include "Editor/EditorWindows/WorldEntitiesWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "Editor/TypeDrawers/ITypeDrawer.h"

#include "World/WorldSystem.h"

#include "Core/Memory.h"
#include "Algorithm/Vector.h"

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
            if (EditorWindowManager::Instance().IsWindowVisable(WorldEntitiesWindow::WINDOW_NAME))
            {
                WorldEntitiesWindow const* entitiesWindow = static_cast<WorldEntitiesWindow const*>(EditorWindowManager::Instance().GetActiveWindow(WorldEntitiesWindow::WINDOW_NAME));
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

        ECS::Entity* EntitiyDescriptionWindow::FindEntity(const Core::GUID& guid)
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
            if (ImGui::Checkbox("Entity Enabled", &isEnabled)) { entity->SetEnabled(isEnabled); }

            for (u32 i = 0; i < entity->GetComponentCount(); ++i)
            {
                ImGui::Spacing();
                ImGui::Spacing();
                DrawComponent(entity->GetComponentByIndex(i));
            }

            if (!m_showAddComponentMenu && ImGui::Button("Add Component"))
            {
                m_showAddComponentMenu = true;
            }

            if (m_showAddComponentMenu)
            {
                if (ImGui::BeginPopupContextItem("Add Component", ImGuiPopupFlags_MouseButtonLeft))
                {
                    std::vector<std::string> componentTypeNames = ECS::ComponentRegistry::GetComponentNames();
                    for (const std::string& componentTypeName : componentTypeNames)
                    {
                        if (ImGui::MenuItem(componentTypeName.c_str()))
                        {
                            entity->AddComponentByName(componentTypeName);
                            m_showAddComponentMenu = false;
                            break;
                        }
                    }
                    ImGui::EndPopup();
                }
            }
        }

        void EntitiyDescriptionWindow::DrawComponent(ECS::Component* component)
        {
            ImGui::Text("%s", component->GetTypeName());
            ImGui::Separator();

            const ITypeDrawer* componentTypeDrawer = TypeDrawerRegister::Instance().GetDrawer(component->GetTypeName());
            if (componentTypeDrawer)
            {
                componentTypeDrawer->Draw(component);
            }
            else
            {
                Reflect::ReflectTypeInfo typeInfo = component->GetTypeInfo();
                for (const Reflect::ReflectTypeMember* member : typeInfo.GetAllMembers())
                {
                    if (!member->IsValid())
                    {
                        continue;
                    }

                    Reflect::ReflectType* memberType = member->GetType();
                    std::vector<Reflect::ReflectType*> memberInheritanceTypes = memberType->GetInheritances();
                    if (Algorithm::VectorFindIf(memberInheritanceTypes, [](const Reflect::ReflectType* type)
                        {
                            return type->GetValueTypeName() == ECS::Component::Type_Name;
                        }) != memberInheritanceTypes.end())
                    {
                        // We have a component.
                        IS_CORE_INFO("");
                    }

                    const ITypeDrawer* typeDrawer = TypeDrawerRegister::Instance().GetDrawer(member->GetType()->GetTypeName().c_str());
                    if (typeDrawer)
                    {
                        //typeDrawer->Draw(member->GetData());
                    }
                }
            }
        }
    }
}