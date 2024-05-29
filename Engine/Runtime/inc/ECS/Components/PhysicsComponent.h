#pragma once

#include "ECS/Entity.h"
#include "Physics/Body.h"
#include "Core/ReferencePtr.h"

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

            Physics::Body& GetPhysicsBody();
            const Physics::Body& GetPhysicsBody() const;

            // Component - Begin

            virtual void OnCreate() override;
            virtual void OnDestroy() override;
            
            // Component - End

        private:
            Ref<Physics::Body> m_physicsBody = nullptr;
        };
    }
}