#pragma once

#include "Physics/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/NonCopyable.h"
#include "Core/RefCount.h"

#ifdef IS_PHYSICS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>
#endif

#include <vector>

namespace Insight::Physics
{
    namespace Jolt
    {
        class PhysicsWorld_Jolt;
    }
    namespace Physx
    {
        class PhysicsWorld_Physx;
    }
    class IShape;

    class IS_PHYSICS Body : public Core::RefCount, public NonCopyable
    {
#ifdef IS_PHYSICS_JOLT
    public:
        Body() = delete;
        Body(JPH::PhysicsSystem* const physicsSystem, JPH::Body* const physicsBody);
        ~Body();

        operator bool() const;
        bool operator==(const Body& other) const;
        bool operator!=(const Body& other) const;

        void SetShape(const IShape* shape);

    private:
        JPH::PhysicsSystem* const m_physicsSystem = nullptr;
        JPH::Body* const m_body = nullptr;
        JPH::BodyID m_bodyId;
#endif

        friend class Jolt::PhysicsWorld_Jolt;
        friend class Physx::PhysicsWorld_Physx;

        template<typename>
        friend struct std::hash;
    };
}

namespace std
{
    template<>
    struct hash<Insight::Physics::Body>
    {
        size_t operator()(const Insight::Physics::Body& bodyId) const noexcept
        {
#ifdef IS_PHYSICS_JOLT
            return hash<JPH::BodyID>{}(bodyId.m_bodyId);
#endif
        }
        size_t operator()(const Insight::Physics::Body* const bodyId) const noexcept
        {
#ifdef IS_PHYSICS_JOLT
            return hash<JPH::BodyID>{}(bodyId->m_bodyId);
#endif
        }
    };
}