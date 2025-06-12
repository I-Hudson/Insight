#include "Editor/TypeDrawers/TypeDrawer_TransformComponent.h"

#include "ECS/Components/TransformComponent.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_TransformComponent::GetTypeName()
        {
            return ECS::TransformComponent::Type_Name;
        }

        void TypeDrawer_TransformComponent::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            ECS::TransformComponent* component = static_cast<ECS::TransformComponent*>(data);

            Maths::Vector3 position = component->GetPosition();
            Maths::Vector3 eularRotationDegress = component->GetRotation().ToEulerDeg();
            Maths::Vector3 scale = component->GetScale();

            if (ImGui::DragFloat3("Position", position.data))
            {
                component->SetPosition(position);
            }

            if (ImGui::DragFloat3("Rotation", eularRotationDegress.data))
            {
                Maths::Quaternion quat = Maths::Quaternion::FromEulerDegress(eularRotationDegress);
                component->SetRotation(quat);
            }

            if (ImGui::DragFloat3("Scale", scale.data))
            {
                component->SetScale(scale);
            }
        }
    }
}