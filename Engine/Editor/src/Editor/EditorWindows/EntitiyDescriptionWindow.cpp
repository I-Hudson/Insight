#include "Editor/EditorWindows/EntitiyDescriptionWindow.h"
#include "Editor/EditorWindows/WorldEntitiesWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Editor/TypeDrawers/TypeDrawerRegister.h"
#include "Editor/TypeDrawers/ITypeDrawer.h"
#include "Editor/EditorGUI.h"

#include "World/WorldSystem.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Core/Memory.h"
#include "Core/StringUtils.h"
#include "Core/Profiler.h"
#include "Algorithm/Vector.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

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
            IS_PROFILE_FUNCTION();
         
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

            std::string entityName = entity->GetName();
            ImGui::InputText("Entity Name:", &entityName);
            entity->SetName(entityName);

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
                    std::vector<const char*> componentTypeNamesCStr;
                    for (const std::string& componentTypeName : componentTypeNames)
                    {
                        componentTypeNamesCStr.push_back(componentTypeName.c_str());
                    }

                    if (ImGui::ListBox("##", &m_addComponentListBoxIndex, componentTypeNamesCStr.data(), static_cast<int>(componentTypeNamesCStr.size())))
                    {
                        entity->AddComponentByName(componentTypeNamesCStr[m_addComponentListBoxIndex]);
                        m_showAddComponentMenu = false;
                        m_addComponentListBoxIndex = 0;
                    }

                    ImGui::EndPopup();
                }

                const Input::InputDevice_KeyboardMouse* inputDevice = Input::InputSystem::Instance().GetKeyboardMouseDevice();
                if (inputDevice->WasReleased(Input::MouseButtons::Left)
                    && !ImGui::IsItemHovered())
                {
                    m_showAddComponentMenu = false;
                }
            }

            if (m_componentContextMenuComponent)
            {
                entity->RemoveComponent(m_componentContextMenuComponent);
                m_componentContextMenuComponent = nullptr;
            }
        }

        void EntitiyDescriptionWindow::DrawComponent(ECS::Component* component)
        {
            ImGui::BeginGroup();

            ImGui::Text("%s", component->GetTypeName());
            std::string entityGuid = component->GetOwnerEntity()->GetGUID().ToString();
            std::string componentGuid = component->GetGuid().ToString();
            std::string payloadData = entityGuid + "::" + componentGuid;
            EditorGUI::ObjectFieldSource("EDW_COMPONENT_DRAG_DROP", payloadData.c_str());

            ImGui::Text("%s", component->GetGuid().ToString().c_str());
            ImGui::Separator();

            const ITypeDrawer* componentTypeDrawer = TypeDrawerRegister::Instance().GetDrawer(component->GetTypeName());
            if (componentTypeDrawer)
            {
                componentTypeDrawer->Draw(component);
            }
            else
            {
                Reflect::TypeInfo typeInfo = component->GetTypeInfo();
                for (const Reflect::MemberInfo& member : typeInfo.GetMemberInfos())
                {
                    Reflect::TypeInfo memberTypeInfo = Reflect::TypeInfoRegistry::GetTypeInfo(member.GetTypeId());
                    if (!member.IsValid())
                    {
                        continue;
                    }
#define IS_EDW_DRAG_DROP 1
#if IS_EDW_DRAG_DROP
                    if (memberTypeInfo.IsDerivedFrom<ECS::Component>()
                        && member.GetValueType() == Reflect::EReflectValueType::Pointer)
                    {
                        void* memberPointer = member.GetMemberPointer();
                        ECS::Component** memberComponentPointer = reinterpret_cast<ECS::Component**>(memberPointer);
                        ECS::Component*& memberComponent = *memberComponentPointer;
                        std::string text = "None";
                        if (memberComponent)
                        {
                            text = memberComponent->GetTypeName();
                        }

                        std::string inputText;
                        if (memberComponent)
                        {
                            inputText = member.GetType().GetPrettyTypeName();
                        }
                        ImGui::InputText(member.GetMemberName().data(), &inputText, ImGuiInputTextFlags_ReadOnly);

                        std::string payloadData;
                        if (EditorGUI::ObjectFieldTarget("EDW_COMPONENT_DRAG_DROP", payloadData, member.GetType()))
                        {
                            std::vector<std::string> splitPayloadData = SplitString(payloadData, '::');

                            Core::GUID entityGuid;
                            entityGuid.StringToGuid(splitPayloadData.at(0));
                            Core::GUID componentGuid;
                            componentGuid.StringToGuid(splitPayloadData.at(1));

                            ECS::Entity* entity = Runtime::WorldSystem::Instance().GetEntityByGUID(entityGuid);
                            if (entity)
                            {
                                ECS::Component* component = entity->GetComponentByGuid(componentGuid);
                                if (component)
                                {

                                    memberComponent = component;
                                }
                            }
                        }
                    }
#endif

                    const ITypeDrawer* typeDrawer = TypeDrawerRegister::Instance().GetDrawer(member.GetType().GetTypeName().data());
                    if (typeDrawer)
                    {
                        typeDrawer->Draw(member.GetMemberPointer());
                    }
                }
            }

            ImGui::EndGroup();

            const Input::InputDevice_KeyboardMouse* inputDevice = Input::InputSystem::Instance().GetKeyboardMouseDevice();

            if (inputDevice->WasReleased(Input::MouseButtons::Right)
                && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
            {
                    m_componentContextMenuGuid = component->GetGuid();
            }

            std::string componentContextMenuId = "##ComponentContextMenu";
            if (m_componentContextMenuGuid.IsValid()
                && m_componentContextMenuGuid == component->GetGuid())
            {
                if (inputDevice->WasReleased(Input::MouseButtons::Left)
                    && !ImGui::IsItemHovered())
                {
                    m_componentContextMenuGuid = Core::GUID::s_InvalidGUID;
                }

                ImGui::OpenPopup(componentContextMenuId.c_str());

                if (ImGui::BeginPopup(componentContextMenuId.c_str()))
                {
                    ImGui::Text(component->GetTypeName());
                    if (ImGui::MenuItem("Remove Component"))
                    {
                        m_componentContextMenuComponent = component;
                    }

                    if (inputDevice->WasReleased(Input::MouseButtons::Left)
                        && !ImGui::IsItemHovered())
                    {
                        m_componentContextMenuGuid = Core::GUID::s_InvalidGUID;
                    }

                    ImGui::EndPopup();
                }
            }
        }
    }
}