#pragma once

#include "Core/TypeAlias.h"
#include "Physics/Defines.h"

#ifdef IS_PHYSICS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
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

    class IS_PHYSICS BodyId
    {
#ifdef IS_PHYSICS_JOLT
    public:
        BodyId(JPH::BodyID bodyId) : JoltBodyId(bodyId) { }

        operator bool() const;
        bool operator==(const BodyId& other) const;
        bool operator!=(const BodyId& other) const;

    private:
        JPH::BodyID JoltBodyId;
#endif

        friend class Jolt::PhysicsWorld_Jolt;
        friend class Physx::PhysicsWorld_Physx;

        template<typename>
        friend struct std::hash;
    };

    //using BodyId = u32;
    //constexpr BodyId c_InvalidBodyId = 0;
}

namespace std
{
    template<>
    struct hash<Insight::Physics::BodyId>
    {
        size_t operator()(const Insight::Physics::BodyId& bodyId) const noexcept
        {
#ifdef IS_PHYSICS_JOLT
            return hash<JPH::BodyID>{}(bodyId.JoltBodyId);
#endif
        }
    };
}