#pragma once

#include "Physics/Defines.h"
#include "Physics/IPhysicsWorld.h"

namespace Insight
{
    namespace Physics
    {
        /// @brief Static interface for physics APU calls.
        class IS_PHYSICS PhysicsWorld
        {
        public:
            PhysicsWorld();
            ~PhysicsWorld();

            static void Initialise();
            static void Shutdown();

            static void Update(const float deltaTime);

            static void StartRecord();
            static void EndRecord();

            /// @brief Add a new body to the world.
            /// @return 
            static BodyId Addbody();
            /// @brief Destroy the body from the physics world. This body must be recreated to use again.
            /// @param bodyId 
            static void DestoryBody(const BodyId bodyId);

            static void ActivateBody(const BodyId body);
            /// @brief Deactivate a body from the physics world, this can be re activated.
            /// @param bodyId 
            static void DeactivateBody(const BodyId bodyId);

        private:
            inline static IPhysicsWorld* s_PhsyicsWorld;
        };
    }
}