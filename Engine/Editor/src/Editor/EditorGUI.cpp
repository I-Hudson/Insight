#include "Editor/EditorGUI.h"

#include "Core/GUID.h"
#include "Core/Logger.h"
#include "Core/StringUtils.h"

#include "ECS/Entity.h"
#include "World/WorldSystem.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight::Editor::EditorGUI
{
    void Editor::EditorGUI::ObjectFieldSource(const char* id, const char* payload, Reflect::Type type)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            std::string payloadData = payload;
            payloadData += ",";
            payloadData += type.GetTypeName();
            payloadData += ",";
            payloadData += std::to_string(type.GetTypeSize());
            ImGui::SetDragDropPayload(id, payloadData.c_str(), payloadData.size());
            ImGui::EndDragDropSource();
        }
    }

    bool ObjectFieldTarget(const char* id, std::string& data, Reflect::Type type)
    {
        data.clear();

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(id);
            if (payload)
            {
                std::string payloadDataString = static_cast<const char*>(payload->Data);
                payloadDataString.resize(payload->DataSize);

                std::vector<std::string> splitStrings = SplitString(payloadDataString, ',');
                std::string& payloadData = splitStrings.at(0);
                std::string& typeName = splitStrings.at(1);
                std::string& typeSize = splitStrings.at(2);

                if (payloadData.empty())
                {
                    // Payload data is empty, we should always be sending something.
                    IS_CORE_WARN("[EditorGUI::ObjectFieldTarget] No payload data was given. Returning.");
                    data.clear();
                    return false;
                }

                if (type.IsValid())
                {
                    Reflect::Type payloadType(typeName, std::stoull(typeSize));
                    if (!payloadType.IsValid())
                    {
                        IS_CORE_WARN("[EditorGUI::ObjectFieldTarget] Type given '{}' is valid, payload type is not valid, no type checking will be performed. Returning.",
                            type.GetTypeName().data());
                        data.clear();
                        return false;
                    }

                    if (type != payloadType)
                    {
                        IS_CORE_WARN("[EditorGUI::ObjectFieldTarget] Type given '{}' is not compatible with payload type '{}'. Returning.",
                            type.GetTypeName().data(), payloadType.GetTypeName().data());
                        data.clear();
                        return false;
                    }
                }

                data = payloadData;
            }
            ImGui::EndDragDropTarget();
        }
        return !data.empty();
    }
}