#include "Physics/Body.h"

#ifdef IS_PHYSICS_JOLT
#include "Physics/Jolt/PhysicsWorld_Jolt.h"
#endif

namespace Insight::Physics
{
#ifdef IS_PHYSICS_JOLT

    Body::Body(JPH::PhysicsSystem* const physicsSystem, JPH::Body* const physicsBody)
        : m_physicsSystem(physicsSystem)
        , m_body(physicsBody)
        , m_bodyId(physicsBody->GetID())
    { }
    Body::~Body()
    { }

    Body::operator bool() const
    {
        return !m_bodyId.IsInvalid();
    }

    bool Body::operator==(const Body& other) const
    {
        return m_bodyId == other.m_bodyId;
    }

    bool Body::operator!=(const Body& other) const
    {
        return !(*this == other);
    }

    void Body::SetShape(const IShape* shape)
    {
        JPH::Ref<JPH::Shape> joltShape = Jolt::PhysicsWorld_Jolt::ShapeToJolt(shape);
        m_physicsSystem->GetBodyInterface().SetShape(m_bodyId, joltShape, true, JPH::EActivation::Activate);
    }
#endif

}