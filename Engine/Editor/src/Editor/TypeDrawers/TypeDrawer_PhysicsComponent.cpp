#include "Editor/TypeDrawers/TypeDrawer_PhysicsComponent.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/Shapes/BoxShape.h"

#include "ECS/Components/PhysicsComponent.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        std::string TypeDrawer_PhysicsComponent::GetTypeName()
        {
            return ECS::PhysicsComponent::Type_Name;
        }

        void TypeDrawer_PhysicsComponent::Draw(void* data, const Reflect::MemberInfo& memberInfo, const std::string_view label) const
        {
            ECS::PhysicsComponent* physicsComponent = static_cast<ECS::PhysicsComponent*>(data);

            Physics::BoxShape* bodyCurrentShape = static_cast<Physics::BoxShape*>(physicsComponent->GetPhysicsBody().GetShape());
            Maths::Vector3& boxShapeExtends = bodyCurrentShape->HalfExtent;

            int moitionType = (int)physicsComponent->GetPhysicsBody().GetMotionType();
            if (ImGui::ListBox("MotionType", &moitionType, Physics::MotionTypeToString, ARRAY_COUNT(Physics::MotionTypeToString)))
            {
                physicsComponent->GetPhysicsBody().SetMotionType((Physics::MotionType)moitionType);
            }

            ImGui::DragFloat3("Box Shape", boxShapeExtends.data);
            if (ImGui::Button("Update Shape"))
            {
                physicsComponent->GetPhysicsBody().SetShape(bodyCurrentShape);
            }
        }
    }
}