#include "Editor/EditorGUI.h"

#include "Core/GUID.h"
#include "Core/Logger.h"

#include "ECS/Entity.h"
#include "World/WorldSystem.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight::Editor::EditorGUI
{
    void IS_EDITOR Editor::EditorGUI::ObjectFieldSource(const char* id, const char* payload)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("EDW_COMPONENT_DRAG_DROP", payload, strlen(payload));
            ImGui::EndDragDropSource();
        }
    }

    void ObjectFieldTarget(const char* id, const char* label, Reflect::Type type, void*& dataToSet)
    {
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(id);
            if (payload)
            {
                std::string payloadDataString = static_cast<const char*>(payload->Data);
                payloadDataString.resize(payload->DataSize);

                std::string entityGuidString = payloadDataString.substr(0, payloadDataString.find("::"));
                std::string componentGuidString = payloadDataString.substr(payloadDataString.find("::") + 2);

                Core::GUID entityGuid;
                entityGuid.StringToGuid(entityGuidString);

                Core::GUID componentGuid;
                componentGuid.StringToGuid(componentGuidString);

                ECS::Entity* entity = Runtime::WorldSystem::Instance().GetActiveWorld()->GetEntityByGUID(entityGuid);
                ECS::Component* component = entity->GetComponentByGuid(componentGuid);

                Reflect::TypeInfo componentTypeInfo;
                bool compatibleWithTarget = false;

                if (component)
                {
                    componentTypeInfo = component->GetTypeInfo();
                    compatibleWithTarget = componentTypeInfo.GetType() == type;
                }

                if (compatibleWithTarget)
                {
                    dataToSet = component;
                }
                else if (component)
                {
                    IS_CORE_INFO("[EntitiyDescriptionWindow::DrawComponent] Target type is '{}', drag object type is '{}'.",
                        type.GetTypeName(), componentTypeInfo.GetTypeId().GetTypeName());
                }
                else
                {
                    IS_CORE_INFO("[EntitiyDescriptionWindow::DrawComponent] Component from GUID '{}' is null.", componentGuid.ToString());
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
}