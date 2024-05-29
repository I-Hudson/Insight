#include "ECS/Components/PhysicsComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/BodyCreationSettings.h"
#include "Physics/ShapeSettings.h"

#include "Core/Asserts.h"

namespace Insight
{
    namespace ECS
    {
        PhysicsComponent::PhysicsComponent()
        { }
        PhysicsComponent::~PhysicsComponent()
        { }

        void PhysicsComponent::OnCreate()
        {
            TransformComponent* transform = GetOwnerEntity()->GetComponent<TransformComponent>();
            ASSERT(transform);

            Physics::BoxShapeSettings shapeSettings(Maths::Vector3(0.5f));
            Physics::BodyCreationSettings bodySettings(&shapeSettings, transform->GetPosition(), transform->GetRotation(), Physics::MotionType::Static, Physics::ObjectLayers::NON_MOVING);
            m_physicsBodyId = Physics::PhysicsWorld::CreateBody(bodySettings);
        }

        void PhysicsComponent::OnDestroy()
        {
            Physics::PhysicsWorld::DestoryBody(m_physicsBodyId);
        }

        IS_SERIALISABLE_CPP(PhysicsComponent);


    }
}