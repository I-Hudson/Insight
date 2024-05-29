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

            float boxShape[3] = { 0,0,0 };
            if (ImGui::DragFloat3("Box Shape", boxShape))
            {
                Physics::BoxShape boxShape(Maths::Vector3(boxShape[0], boxShape[1], boxShape[2]));
                Physics::PhysicsWorld::UpdateBodyShape(physicsComponent->GetPhysicsBodyId(), &boxShape);
            }
        }
    }
}