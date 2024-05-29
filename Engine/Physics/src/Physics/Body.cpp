#include "Physics/Body.h"
#include "Core/Logger.h"

#ifdef IS_PHYSICS_JOLT
#include "Physics/Jolt/PhysicsWorld_Jolt.h"
#endif

namespace Insight::Physics
{
#ifdef IS_PHYSICS_JOLT
    Body::Body(JPH::PhysicsSystem* const physicsSystem, JPH::Body* const physicsBody, IShape* shape)
        : m_physicsSystem(physicsSystem)
        , m_body(physicsBody)
        , m_bodyId(physicsBody->GetID())
        , m_shape(shape)
    { }
#elif defined(IS_PHYSICS_PHYSX)
#endif

    Body::~Body()
    { }

    Body::operator bool() const
    {
#ifdef IS_PHYSICS_JOLT
        return !m_bodyId.IsInvalid();
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    bool Body::operator==(const Body& other) const
    {
#ifdef IS_PHYSICS_JOLT
        return m_bodyId == other.m_bodyId;
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    bool Body::operator!=(const Body& other) const
    {
#ifdef IS_PHYSICS_JOLT
        return !(*this == other);
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    IShape* Body::GetShape()
    {
        ASSERT(m_shape);
        return m_shape.Ptr();
    }
    const IShape* Body::GetShape() const
    {
        ASSERT(m_shape);
        return m_shape.Ptr();
    }
    void Body::SetShape(const Ref<IShape>& shape)
    {
#ifdef IS_PHYSICS_JOLT
        if (!shape)
        {
            IS_LOG_CORE_ERROR("[Body::SetShape] 'shape' is a nullptr.");
            return;
        }

        m_shape = shape;
        JPH::Ref<JPH::Shape> joltShape = Jolt::PhysicsWorld_Jolt::ShapeToJolt(m_shape.Ptr());
        m_physicsSystem->GetBodyInterface().SetShape(m_bodyId, joltShape, true, JPH::EActivation::Activate);
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }
}