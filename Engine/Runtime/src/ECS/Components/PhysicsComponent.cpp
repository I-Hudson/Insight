#include "ECS/Components/PhysicsComponent.h"
#include "ECS/Components/TransformComponent.h"

#include "Physics/PhysicsWorld.h"
#include "Physics/BodyCreationSettings.h"
#include "Physics/Shapes/BoxShape.h"

#include "Core/Asserts.h"

namespace Insight
{
    namespace ECS
    {
        PhysicsComponent::PhysicsComponent()
        { }
        PhysicsComponent::~PhysicsComponent()
        { }

        IS_SERIALISABLE_CPP(PhysicsComponent);

        Physics::Body& PhysicsComponent::GetPhysicsBodyId()
        {
            return *m_physicsBody.Ptr();
        }
        const Physics::Body& PhysicsComponent::GetPhysicsBodyId() const
        {
            return *m_physicsBody.Ptr();
        }

        void PhysicsComponent::OnCreate()
        {
            TransformComponent* transform = GetOwnerEntity()->GetComponent<TransformComponent>();
            ASSERT(transform);

            Ref<Physics::BoxShape> boxShape = ::New<Physics::BoxShape>(Maths::Vector3(0.5f));
            Physics::BodyCreationSettings bodySettings(boxShape, transform->GetPosition(), transform->GetRotation(), Physics::MotionType::Static, Physics::ObjectLayers::NON_MOVING);
            m_physicsBody = Physics::PhysicsWorld::CreateBody(bodySettings);
        }

        void PhysicsComponent::OnDestroy()
        {
            Physics::PhysicsWorld::DestoryBody(m_physicsBody);
        }

    }
}