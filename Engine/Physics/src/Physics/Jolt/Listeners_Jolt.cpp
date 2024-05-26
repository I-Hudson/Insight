#ifdef IS_PHYSICS_JOLT
#include "Physics/Jolt/Listeners_Jolt.h"

#include <iostream>

namespace Insight::Physics
{
	//===========================
	// MyContactListener
	//===========================
	JPH::ValidateResult	ContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
	{
		std::cout << "Contact validate callback" << std::endl;
	
		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}
	
	void ContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		std::cout << "A contact was added" << std::endl;
	}
	
	void ContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		std::cout << "A contact was persisted" << std::endl;
	}
	
	void ContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
	{
		std::cout << "A contact was removed" << std::endl;
	}

	//===========================
	// MyBodyActivationListener
	//===========================
	void BodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
	{
		std::cout << "A body got activated" << std::endl;
	}
	
	void BodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
	{
		std::cout << "A body went to sleep" << std::endl;
	}
}
#endif