#pragma once
#ifdef IS_PHYSICS_JOLT

#include "Physics/IPhysicsWorld.h"

#include "Physics/Jolt/ObjectLayerFilters_Jolt.h"
#include "Physics/Jolt/Listeners_Jolt.h"

#include "Core/Asserts.h"

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
        virtual BodyId Addbody() override;
        /// @brief Destroy the body from the physics world. This body must be recreated to use again.
        /// @param bodyId 
        virtual void DestoryBody(const BodyId bodyId) override;

        virtual void ActivateBody(const BodyId body) override;
        /// @brief Deactivate a body from the physics world, this can be re activated.
        /// @param bodyId 
        virtual void DeactivateBody(const BodyId bodyId) override;

    private:
        inline static JPH::PhysicsSystem m_physicsSystem;
        inline static JPH::JobSystemThreadPool m_jobSystem;
        inline static JPH::TempAllocatorMalloc m_tempAllocatorMalloc;
        inline static std::vector<JPH::BodyID> m_bodyIds;

        // Create class that filters object vs object layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        inline static ObjectLayerPairFilter m_objectLayerPairFilter;
        // Create class that filters object vs broadphase layers
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        inline static ObjectVsBroadPhaseLayerFilter m_objectVsBroadPhaseLayerFilter;
        // Create mapping table from object layer to broadphase layer
        // Note: As this is an interface, PhysicsSystem will take a reference to this so this instance needs to stay alive!
        inline static BPLayerInterface m_BPLayerInterface;

        inline static ContactListener m_contactListener;
        inline static BodyActivationListener m_bodyActivationListener;

        //std::vector<void> m_bodiesToAdd;
        //std::vector<void> m_bodiesToRemove;

#ifdef JPH_DEBUG_RENDERER
        inline static bool m_isRecording = false;
        inline static PhysicsStreamOut_Jolt m_recorderStreamOut;
        inline static JPH::DebugRendererRecorder* m_debugRendererRecorder;
#endif
    };
}
#endif