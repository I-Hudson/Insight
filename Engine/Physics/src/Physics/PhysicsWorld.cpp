#include "Physics/PhysicsWorld.h"

#ifdef IS_PHYSICS_PHYSX
//#include "Physics/Jolt/PhysicsWorld_Physx.h"
#endif
#ifdef IS_PHYSICS_JOLT
#include "Physics/Jolt/PhysicsWorld_Jolt.h"
#endif

#include "Core/Memory.h"

namespace Insight
{
    namespace Physics
    {
        PhysicsWorld::PhysicsWorld()
        { }

        PhysicsWorld::~PhysicsWorld()
        { }

		void PhysicsWorld::Initialise()
		{
#ifdef IS_PHYSICS_PHYSX
			//s_PhsyicsWorld = ::New<Jolt::PhysicsWorld_Physx>();
#elif defined(IS_PHYSICS_JOLT)
			s_PhsyicsWorld = ::New<Jolt::PhysicsWorld_Jolt>();
#endif
		}

		void PhysicsWorld::Shutdown()
		{
			s_PhsyicsWorld->Shutdown();
			Delete(s_PhsyicsWorld);
		}

		void PhysicsWorld::Update(const float deltaTime)
		{
			s_PhsyicsWorld->Update(deltaTime);
		}

		void PhysicsWorld::StartRecord()
		{
			s_PhsyicsWorld->StartRecord();
		}

		void PhysicsWorld::EndRecord()
		{
			s_PhsyicsWorld->EndRecord();
		}

		BodyId PhysicsWorld::Addbody()
		{
			return s_PhsyicsWorld->Addbody();
		}

		void PhysicsWorld::DestoryBody(const BodyId bodyId)
		{
			s_PhsyicsWorld->DestoryBody(bodyId);
		}

		void PhysicsWorld::ActivateBody(const BodyId body)
		{
			s_PhsyicsWorld->ActivateBody(body);

		}

		void PhysicsWorld::DeactivateBody(const BodyId bodyId)
		{
			s_PhsyicsWorld->DeactivateBody(bodyId);
		}
    }
}
