#pragma once
#ifdef IS_PHYSICS_PHYSX

#include "Physics/IPhysicsWorld.h"

namespace Insight::Physics::Physx
{
    class PhysicsWorld_Physx : public IPhysicsWorld
    {
        PhysicsWorld_Physx();
        ~PhysicsWorld_Physx() override;

        virtual void Initialise() override;
        virtual void Shutdown() override;

        virtual void Update(const float deltaTime) override;

        virtual void StartRecord() override;
        virtual void EndRecord() override;

        /// @brief Add a new body to the world.
        /// @return 
        virtual BodyId Addbody() override;
        /// @brief Destroy the body from the physics world. This body must be recreated to use again.
        /// @param bodyId 
        virtual void DestoryBody(const BodyId bodyId) override;

        virtual void ActivateBody(const BodyId body) override;
        /// @brief Deactivate a body from the physics world, this can be re activated.
        /// @param bodyId 
        virtual void DeactivateBody(const BodyId bodyId) override;
    };
}
#endif