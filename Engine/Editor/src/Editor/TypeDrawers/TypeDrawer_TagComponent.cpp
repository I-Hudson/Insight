#include "Editor/TypeDrawers/TypeDrawer_TagComponent.h"

#include "ECS/Components/TagComponent.h"
#include "IconsFontAwesome5.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_TagComponent::GetTypeName()
        {
            return ECS::TagComponent::Type_Name;
        }

        void TypeDrawer_TagComponent::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            ECS::TagComponent* tagComponentPointer = static_cast<ECS::TagComponent*>(data);
            ECS::TagComponent& tagComponent = *tagComponentPointer;

            for (auto& tag : tagComponentPointer->GetAllTags())
            {
                std::string uniqueId = "##" + std::to_string(*(u64*)&tag);
                ImGui::InputText(uniqueId.c_str(), &tag);
            }

            if (ImGui::Button("+"))
            {
                tagComponent.AddTag("");
            }
            ImGui::SameLine();
            if (ImGui::Button("-"))
            {

            }
        }
    }
}