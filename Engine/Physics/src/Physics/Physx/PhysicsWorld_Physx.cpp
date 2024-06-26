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

    BodyId PhysicsWorld_Physx::CreateBody()
    {
        return BodyId();
    }

    void PhysicsWorld_Physx::DestoryBody(const BodyId bodyId)
    {
    }

    void PhysicsWorld_Physx::AddBody(const BodyId body)
    {
    }

    void PhysicsWorld_Physx::RemoveBody(const BodyId bodyId)
    {
    }
}

#endif