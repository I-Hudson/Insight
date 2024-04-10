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
    namespace Internal
    {
        bool VerifyObjectFieldPayload(std::string& payload, Reflect::Type type)
        {
            std::vector<std::string> splitStrings = SplitString(payload, ',');
            std::string& payloadData = splitStrings.at(0);
            std::string& typeName = splitStrings.at(1);
            std::string& typeSize = splitStrings.at(2);

            payload.clear();
            if (payloadData.empty())
            {
                // Payload data is empty, we should always be sending something.
                IS_CORE_WARN("[EditorGUI::VerifyObjectFieldPayload] No payload data was given. Returning.");
                return false;
            }

            if (type.IsValid())
            {
                Reflect::Type payloadType(typeName, std::stoull(typeSize));
                if (!payloadType.IsValid())
                {
                    IS_CORE_WARN("[EditorGUI::VerifyObjectFieldPayload] Type given '{}' is valid, payload type is not valid, no type checking will be performed. Returning.",
                        type.GetTypeName().data());
                    return false;
                }

                if (type != payloadType)
                {
                    IS_CORE_WARN("[EditorGUI::VerifyObjectFieldPayload] Type given '{}' is not compatible with payload type '{}'. Returning.",
                        type.GetTypeName().data(), payloadType.GetTypeName().data());
                    return false;
                }
            }
            payload = std::move(payloadData);
            return true;
        }
    }


    void ObjectFieldSource(const char* id, const char* payload, Reflect::Type type)
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

                if (Internal::VerifyObjectFieldPayload(payloadDataString, type))
                {
                    data = std::move(payloadDataString);
                }
            }
            ImGui::EndDragDropTarget();
        }
        return !data.empty();
    }

    bool IS_EDITOR ObjectFieldTargetCustomRect(const char* id, const ImRect& rect, std::string& data, Reflect::Type type)
    {
        data.clear();

        if (ImGui::BeginDragDropTargetCustom(rect, ImGuiID(id)))
        {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(id);
            if (payload)
            {
                std::string payloadDataString = static_cast<const char*>(payload->Data);
                payloadDataString.resize(payload->DataSize);

                if (Internal::VerifyObjectFieldPayload(payloadDataString, type))
                {
                    data = std::move(payloadDataString);
                }
            }
            ImGui::EndDragDropTarget();
        }
        return !data.empty();
    }
}