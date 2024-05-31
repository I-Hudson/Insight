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

    Maths::Vector3 Body::GetPosition() const
    {
#ifdef IS_PHYSICS_JOLT
        JPH::Vec3 joltPosition = m_body->GetPosition();
        return Maths::Vector3(joltPosition.GetX(), joltPosition.GetY(), joltPosition.GetZ());
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    Maths::Quaternion Body::GetRotation() const
    {
#ifdef IS_PHYSICS_JOLT
        JPH::Quat joltRotation = m_body->GetRotation();
        return Maths::Quaternion(joltRotation.GetW(), joltRotation.GetX(), joltRotation.GetY(), joltRotation.GetZ());
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    Maths::Matrix4 Body::GetWorldTransform() const
    {
#ifdef IS_PHYSICS_JOLT
        JPH::RMat44 joltTransform = m_body->GetWorldTransform();
        return Maths::Matrix4(
            joltTransform.GetColumn4(0).GetX(), joltTransform.GetColumn4(0).GetY(), joltTransform.GetColumn4(0).GetZ(), joltTransform.GetColumn4(0).GetW(),
            joltTransform.GetColumn4(1).GetX(), joltTransform.GetColumn4(1).GetY(), joltTransform.GetColumn4(1).GetZ(), joltTransform.GetColumn4(1).GetW(),
            joltTransform.GetColumn4(2).GetX(), joltTransform.GetColumn4(2).GetY(), joltTransform.GetColumn4(2).GetZ(), joltTransform.GetColumn4(2).GetW(),
            joltTransform.GetColumn4(3).GetX(), joltTransform.GetColumn4(3).GetY(), joltTransform.GetColumn4(3).GetZ(), joltTransform.GetColumn4(3).GetW());
#elif defined(IS_PHYSICS_PHYSX)
#endif
    }

    void Body::SetMotionType(const MotionType motionType)
    {
        if (m_motionType != motionType)
        {
            m_motionType = motionType;
            JPH::BodyInterface& bodyInterface = m_physicsSystem->GetBodyInterface();
            bodyInterface.DeactivateBody(m_bodyId);
            bodyInterface.SetMotionType(m_bodyId, Jolt::PhysicsWorld_Jolt::MotionTypeToJolt(m_motionType), JPH::EActivation::Activate);
            if (m_motionType == MotionType::Dynamic)
            {
                bodyInterface.SetObjectLayer(m_bodyId, ObjectLayers::MOVING);
            }
            else
            {
                bodyInterface.SetObjectLayer(m_bodyId, ObjectLayers::NON_MOVING);
            }
            bodyInterface.ActivateBody(m_bodyId);
        }
    }
    MotionType Body::GetMotionType() const
    {
        return m_motionType;
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