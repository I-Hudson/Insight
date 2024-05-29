#ifdef IS_PHYSICS_JOLT
#include "Physics/Jolt/PhysicsWorld_Jolt.h"
#include "Physics/BodyCreationSettings.h"
#include "Physics/Shapes/BoxShape.h"

#include "Core/Memory.h"
#include "Core/Profiler.h"
#include "FileSystem/FileSystem.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include <iostream>
#include <iomanip>

JPH_SUPPRESS_WARNING_PUSH
namespace Insight::Physics::Jolt
{
	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message
		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		// Print to the TTY
		std::cout << buffer << std::endl;
	}

#ifdef JPH_ENABLE_ASSERTS

	// Callback for asserts, connect this to your own assert handler if you have one
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, u32 inLine)
	{
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

		// Breakpoint
		return true;
	};

#endif // JPH_ENABLE_ASSERTS

	// Put the 'real' operator in a namespace so that users can opt in to use it:
	// using namespace JPH::literals;
	constexpr JPH::Real operator ""_r(long double inValue) { return JPH::Real(inValue); }

	PhysicsWorld_Jolt::PhysicsWorld_Jolt()
	{ }
	PhysicsWorld_Jolt::~PhysicsWorld_Jolt()
	{ }

	void PhysicsWorld_Jolt::Initialise()
    {
		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
			// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();

		// Install trace and assert callbacks
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		JPH::Factory::sInstance = ::New<JPH::Factory>();

		// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		JPH::RegisterTypes();

		// We need a temp allocator for temporary allocations during the physics update. We're
		// pre-allocating 10 MB to avoid having to do allocations during the physics update.
		// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		//JPH::TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
		//JPH::TempAllocatorMalloc tempAllocatorMalloc;
		m_tempAllocatorMalloc.Allocate(0);

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		m_jobSystem.Init(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const u32 cMaxBodies = 65536;

		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		const u32 cNumBodyMutexes = 0;

		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const u32 cMaxBodyPairs = 65536;

		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
		const u32 cMaxContactConstraints = 10240;

		// Now we can create the actual physics system.
		m_physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_BPLayerInterface, m_objectVsBroadPhaseLayerFilter, m_objectLayerPairFilter);

		// A body activation listener gets notified when bodies activate and go to sleep
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//JPH::MyBodyActivationListener body_activation_listener;
		m_physicsSystem.SetBodyActivationListener(&m_bodyActivationListener);

		// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
		// Note that this is called from a job so whatever you do here needs to be thread safe.
		// Registering one is entirely optional.
		//JPH::MyContactListener contact_listener;
		m_physicsSystem.SetContactListener(&m_contactListener);

		// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
		// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
		JPH::BodyInterface& body_interface = m_physicsSystem.GetBodyInterface();

		// Next we can create a rigid body to serve as the floor, we make a large box
		// Create the settings for the collision volume (the shape).
		// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
		//JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
		//floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

		// Create the shape
		//JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
		//JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		//JPH::BodyCreationSettings floor_settings(floor_shape, JPH::RVec3(0.0_r, -1.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

		// Create the actual rigid body
		//JPH::Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
		//body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

		// Now create a dynamic body to bounce on the floor
		// Note that this uses the shorthand version of creating and adding a body to the world
		//JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f), JPH::RVec3(0.0_r, 2.0_r, 0.0_r), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
		//JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);

		// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
		// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
		//body_interface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));

		Ref<BoxShape> boxShape = ::New<BoxShape>(Maths::Vector3(100.0f, 1.0f, 100.0f));
		BodyCreationSettings bodyCreateSettings(boxShape, Maths::Vector3(0), Maths::Quaternion::Identity, MotionType::Static, ObjectLayers::NON_MOVING);
		//CreateBody(bodyCreateSettings);

		boxShape = ::New<BoxShape>(Maths::Vector3(5.0f, 1.0f, 5.0f));
		bodyCreateSettings = BodyCreationSettings(boxShape, Maths::Vector3(0.0f, 20.0f, 0.0f), Maths::Quaternion::Identity, MotionType::Dynamic, ObjectLayers::MOVING);
		//CreateBody(bodyCreateSettings);

		//StartRecord();
    }

	void PhysicsWorld_Jolt::Shutdown()
	{
		JPH::BodyInterface& body_interface = m_physicsSystem.GetBodyInterface();

		// Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
		// Destroy the sphere. After this the sphere ID is no longer valid.
		// Remove and destroy the floor
		std::unique_lock bodiesLock(m_bodiesMutex);
		if (m_bodies.size())
		{
			std::vector<JPH::BodyID> bodiesToRemoveAndDestroy;
			for (const Ref<Body>& body : m_bodies)
			{
				ASSERT(body->GetReferenceCount() == 1);
				bodiesToRemoveAndDestroy.push_back(body->m_bodyId);
			}
			body_interface.RemoveBodies(bodiesToRemoveAndDestroy.data(), static_cast<int>(bodiesToRemoveAndDestroy.size()));
			body_interface.DestroyBodies(bodiesToRemoveAndDestroy.data(), static_cast<int>(bodiesToRemoveAndDestroy.size()));
			m_bodies.clear();
		}
		bodiesLock.unlock();

		// Unregisters all types with the factory and cleans up the default material
		JPH::UnregisterTypes();

		// Destroy the factory
		Delete(JPH::Factory::sInstance);
		JPH::Factory::sInstance = nullptr;

		EndRecord();
	}

	void PhysicsWorld_Jolt::Update(const float deltaTime)
	{
		IS_PROFILE_FUNCTION();

		std::unique_lock l(m_bodiesMutex);
		const bool startRecord = !m_bodies.empty();
		l.unlock();
		if (startRecord)
		{
			StartRecord();
		}


		// We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics system.
		const float cDeltaTime = 1.0f / 60.0f;

		// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
		// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
		// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
		{
			IS_PROFILE_SCOPE("OptimizeBroadPhase");
			m_physicsSystem.OptimizeBroadPhase();
		}

		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = 1;
		// Step the world
		{
			IS_PROFILE_SCOPE("PhysicsSystem Update");
			m_physicsSystem.Update(cDeltaTime, cCollisionSteps, &m_tempAllocatorMalloc, &m_jobSystem);
		}

#ifdef JPH_DEBUG_RENDERER
		if (m_isRecording)
		{
			JPH::BodyManager::DrawSettings drawSettings;
			m_physicsSystem.DrawBodies(drawSettings, m_debugRendererRecorder);
			m_physicsSystem.DrawConstraints(m_debugRendererRecorder);
			m_physicsSystem.DrawConstraintLimits(m_debugRendererRecorder);
			m_physicsSystem.DrawConstraintReferenceFrame(m_debugRendererRecorder);

			m_debugRendererRecorder->EndFrame();
		}
#endif
	}

	void PhysicsWorld_Jolt::StartRecord()
	{
#ifdef JPH_DEBUG_RENDERER
		m_isRecording = true;
		if (!m_debugRendererRecorder)
		{
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

			std::ostringstream oss;
			oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
			auto str = oss.str();

			FileSystem::CreateFolder("Physics/");
			m_recorderStreamOut.OpenFile("Physics/Physics_" + str + ".bin");

			m_debugRendererRecorder = New<JPH::DebugRendererRecorder>(m_recorderStreamOut);
		}
#endif
	}

	void PhysicsWorld_Jolt::EndRecord()
	{
#ifdef JPH_DEBUG_RENDERER
		if (m_isRecording)
		{
			m_isRecording = false;
			Delete(m_debugRendererRecorder);
			m_debugRendererRecorder = nullptr;

			m_recorderStreamOut.Close();
		}
#endif
	}

	Ref<Body> PhysicsWorld_Jolt::CreateBody(const BodyCreationSettings& bodyCreationSettings)
	{
		JPH::ShapeSettings* nullShapeSettings = nullptr;
		JPH::BodyCreationSettings settings(nullShapeSettings,
			Vector3ToJolt(bodyCreationSettings.m_position), 
			QuaterianToJolt(bodyCreationSettings.m_rotation),
			MotionTypeToJolt(bodyCreationSettings.m_motionType),
			bodyCreationSettings.m_objectLayer);

		JPH::Ref<JPH::Shape> joltShape = ShapeToJolt(bodyCreationSettings.Shape.Ptr());
		ASSERT(joltShape.GetPtr() != nullptr);
		settings.SetShape(joltShape);

		JPH::Body* joltBody = m_physicsSystem.GetBodyInterface().CreateBody(settings);

		Ref<Body> body = ::New<Body>(&m_physicsSystem, joltBody);
		{
			std::lock_guard l(m_bodiesMutex);
			m_bodies.insert(body);
		}

		AddBody(body);

		return body;
	}

	void PhysicsWorld_Jolt::DestoryBody(const Ref<Body>& body)
	{
		ASSERT(HasBody(body));
		RemoveBody(body);
		m_physicsSystem.GetBodyInterface().DestroyBody(body->m_bodyId);
		{
			std::lock_guard l(m_bodiesMutex);
			m_bodies.erase(body);
		}
	}

	void PhysicsWorld_Jolt::AddBody(const Ref<Body>& body)
	{
		ASSERT(HasBody(body));
		m_physicsSystem.GetBodyInterface().AddBody(body->m_bodyId, JPH::EActivation::Activate);
	}

	void PhysicsWorld_Jolt::RemoveBody(const Ref<Body>& body)
	{
		ASSERT(HasBody(body));
		m_physicsSystem.GetBodyInterface().RemoveBody(body->m_bodyId);
	}

	bool PhysicsWorld_Jolt::HasBody(const Ref<Body>& body) const
	{
		std::lock_guard l(m_bodiesMutex);
		return m_bodies.find(body) != m_bodies.end();
	}

	JPH::EMotionType PhysicsWorld_Jolt::MotionTypeToJolt(const MotionType motionType)
	{
		switch (motionType)
		{
		case MotionType::Static: return JPH::EMotionType::Static;
		case MotionType::Dynamic: return JPH::EMotionType::Dynamic;
		case MotionType::Kinematic: return JPH::EMotionType::Kinematic;
		}
		FAIL_ASSERT();
		return JPH::EMotionType();
	}

	JPH::Quat PhysicsWorld_Jolt::QuaterianToJolt(const Maths::Quaternion& quat)
	{
		return JPH::Quat(quat.x, quat.y, quat.z, quat.w);
	}

	JPH::Vec3 PhysicsWorld_Jolt::Vector3ToJolt(const Maths::Vector3& vec)
	{
		return JPH::Vec3(vec.x, vec.y, vec.z);
	}

	JPH::Vec4 PhysicsWorld_Jolt::Vector4ToJolt(const Maths::Vector4& vec)
	{
		return JPH::Vec4(vec.x, vec.y, vec.z, vec.w);
	}

	JPH::Ref<JPH::Shape> PhysicsWorld_Jolt::ShapeToJolt(const IShape* shape)
	{
		switch (shape->ShapeSubType)
		{
		case ShapeSubTypes::Box:
		{
			const BoxShape* boxShape = static_cast<const BoxShape*>(shape);
			JPH::BoxShapeSettings joltBoxShapeSettings(Vector3ToJolt(boxShape->HalfExtent), boxShape->ConvexRadius, nullptr);
			return joltBoxShapeSettings.Create().Get();
		}
		default:
		{
			FAIL_ASSERT();
			break;
		}
		}
		FAIL_ASSERT();
		return nullptr;
	}
}
JPH_SUPPRESS_WARNING_POP
#endif