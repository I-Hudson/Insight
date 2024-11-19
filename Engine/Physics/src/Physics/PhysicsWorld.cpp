#include "Physics/PhysicsWorld.h"
#include "Core/Asserts.h"

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
			if (s_PhsyicsWorld)
			{
				return;
			}

#ifdef IS_PHYSICS_PHYSX
			//s_PhsyicsWorld = ::New<Jolt::PhysicsWorld_Physx>();
#elif defined(IS_PHYSICS_JOLT)
			s_PhsyicsWorld = ::New<Jolt::PhysicsWorld_Jolt>();
#endif

			ASSERT(s_PhsyicsWorld);
			s_PhsyicsWorld->Initialise();
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

		Ref<Body> PhysicsWorld::CreateBody(const BodyCreationSettings& bodyCreationSettings)
		{
			return s_PhsyicsWorld->CreateBody(bodyCreationSettings);
		}

		void PhysicsWorld::DestoryBody(const Ref<Body>& body)
		{
			s_PhsyicsWorld->DestoryBody(body);
		}

		void PhysicsWorld::AddBody(const Ref<Body>& body)
		{
			s_PhsyicsWorld->AddBody(body);

		}

		void PhysicsWorld::RemoveBody(const Ref<Body>& body)
		{
			s_PhsyicsWorld->RemoveBody(body);
		}

		const DebugRendererData& PhysicsWorld::GetDebugRenderData()
		{
			return s_PhsyicsWorld->GetDebugRenderData();
		}
    }
}
