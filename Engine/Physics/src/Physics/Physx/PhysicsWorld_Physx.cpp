#ifdef IS_PHYSICS_PHYSX
#include "Physics/Physx/PhysicsWorld_Physx.h"

namespace Insight::Physics::Physx
{
    PhysicsWorld_Physx::PhysicsWorld_Physx()
    { }

    PhysicsWorld_Physx::~PhysicsWorld_Physx()
    { }

    void PhysicsWorld_Physx::Initialise()
    {
    }

    void PhysicsWorld_Physx::Shutdown()
    {
    }

    void PhysicsWorld_Physx::Update(const float deltaTime)
    {
    }

    void PhysicsWorld_Physx::StartRecord()
    {
    }

    void PhysicsWorld_Physx::EndRecord()
    {
    }

    BodyId PhysicsWorld_Physx::Addbody()
    {
        return BodyId();
    }

    void PhysicsWorld_Physx::DestoryBody(const BodyId bodyId)
    {
    }

    void PhysicsWorld_Physx::ActivateBody(const BodyId body)
    {
    }

    void PhysicsWorld_Physx::DeactivateBody(const BodyId bodyId)
    {
    }
}

#endif