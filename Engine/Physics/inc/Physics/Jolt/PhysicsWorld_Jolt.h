#pragma once
#ifdef IS_PHYSICS_JOLT

#include "Physics/IPhysicsWorld.h"

#include "Physics/Jolt/ObjectLayerFilters_Jolt.h"
#include "Physics/Jolt/Listeners_Jolt.h"
#include "Physics/MotionType.h"

#include "Core/Asserts.h"
#include "Maths/Vector3.h"
#include "Maths/Vector4.h"
#include "Maths/Quaternion.h"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Renderer/DebugRendererRecorder.h>

#include <fstream>
#include <mutex>

namespace Insight::Physics::Jolt
{
    class PhysicsStreamOut_Jolt : public JPH::StreamOut
    {
    public:
        void OpenFile(std::string fileName)
        {
            ASSERT(!m_file.is_open());
            m_file.open(fileName, std::ios::binary | std::ios::out);
        }

        void Close()
        {
            if (m_file.is_open())
            {
                m_file.close();
            }
        }

        /// Write a string of bytes to the binary stream
        virtual void    WriteBytes(const void* inData, size_t inNumBytes)
        {
            m_file.write((const char*)inData, inNumBytes);
        }

        /// Returns true if there was an IO failure
        virtual bool IsFailed() const
        {
            return !m_file.is_open();
        }

    private:
        std::ofstream m_file;
    };

    class PhysicsWorld_Jolt : public IPhysicsWorld
    {
    public:
        PhysicsWorld_Jolt();
        ~PhysicsWorld_Jolt() override;

        virtual void Initialise() override;
        virtual void Shutdown() override;

        virtual void Update(const float deltaTime) override;

        virtual void StartRecord() override;
        virtual void EndRecord() override;

        /// @brief Add a new body to the world.
        /// @return 
        virtual BodyId CreateBody(const BodyCreationSettings& bodyCreationSettings) override;
        /// @brief Destroy the body from the physics world. This body must be recreated to use again.
        /// @param bodyId 
        virtual void DestoryBody(const BodyId bodyId) override;

        virtual void AddBody(const BodyId bodyId) override;
        /// @brief Deactivate a body from the physics world, this can be re activated.
        /// @param bodyId 
        virtual void RemoveBody(const BodyId bodyId) override;

    private:
        bool HasBody(const BodyId bodyId) const;

        JPH::EMotionType MotionTypeToJolt(const MotionType motionType) const;

        JPH::Quat QuaterianToJolt(const Maths::Quaternion& quat) const;
        JPH::Vec3 Vector3ToJolt(const Maths::Vector3& vec) const;
        JPH::Vec4 Vector4ToJolt(const Maths::Vector4& vec) const;

    private:
        JPH::PhysicsSystem m_physicsSystem;
        JPH::JobSystemThreadPool m_jobSystem;
        JPH::TempAllocatorMalloc m_tempAllocatorMalloc;
        std::vector<JPH::BodyID> m_bodyIds;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        ObjectLayerPairFilter m_objectLayerPairFilter;
        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        ObjectVsBroadPhaseLayerFilter m_objectVsBroadPhaseLayerFilter;
        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        BPLayerInterface m_BPLayerInterface;

        ContactListener m_contactListener;
        BodyActivationListener m_bodyActivationListener;

        std::unordered_map<BodyId, JPH::Body*> m_bodies;
        mutable std::mutex m_bodiesMutex;

#ifdef JPH_DEBUG_RENDERER
        bool m_isRecording = false;
        PhysicsStreamOut_Jolt m_recorderStreamOut;
        JPH::DebugRendererRecorder* m_debugRendererRecorder = nullptr;
#endif
    };
}
#endif