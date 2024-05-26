#pragma once

#include "ECS/Entity.h"
#include "Physics/BodyId.h"

#include "Generated/PhysicsComponent_reflect_generated.h"

namespace Insight
{
    namespace ECS
    {
        REFLECT_CLASS();
        class IS_RUNTIME PhysicsComponent : public Component
        {
            REFLECT_GENERATED_BODY();
        public:
            PhysicsComponent();
            virtual ~PhysicsComponent() override;

            IS_COMPONENT(PhysicsComponent);
            IS_SERIALISABLE_H(PhysicsComponent);

            // Component - Begin

            virtual void OnCreate() override;
            virtual void OnDestroy() override;
            
            // Component - End

        private:
            Physics::BodyId m_physicsBodyId = Physics::c_InvalidBodyId;
        };
    }
}