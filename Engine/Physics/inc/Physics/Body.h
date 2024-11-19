#pragma once

#include "Physics/Defines.h"
#include "Physics/Shapes/Shape.h"
#include "Physics/MotionType.h"

#include "Core/TypeAlias.h"
#include "Core/NonCopyable.h"
#include "Core/RefCount.h"
#include "Core/ReferencePtr.h"

#include "Maths/Matrix4.h"

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
    public:
        ~Body();

        operator bool() const;
        bool operator==(const Body& other) const;
        bool operator!=(const Body& other) const;

        /// @brief World space position of the body
        /// @return Vector3
        Maths::Vector3 GetPosition() const;
        /// @brief World space rotation of the body
        /// @return Quaternion
        Maths::Quaternion GetRotation() const;
        /// @brief Calculates the transform of this body
        /// @return Matrix4
        Maths::Matrix4 GetWorldTransform() const;

        void SetMotionType(const MotionType motionType);
        MotionType GetMotionType() const;

        IShape* GetShape();
        const IShape* GetShape() const;
        void SetShape(const Ref<IShape>& shape);

#ifdef IS_PHYSICS_JOLT
        Body(JPH::PhysicsSystem* const physicsSystem, JPH::Body* const physicsBody, IShape* shape);
    private:
        JPH::PhysicsSystem* const m_physicsSystem = nullptr;
        JPH::Body* const m_body = nullptr;
        JPH::BodyID m_bodyId;
#elif defined(IS_PHYSICS_PHYSX)
#else
        Body();
#endif

    private:
        Ref<IShape> m_shape = nullptr;
        MotionType m_motionType = MotionType::Static;

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