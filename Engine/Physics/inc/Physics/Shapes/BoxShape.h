#pragma once

#include "Physics/Shapes/Shape.h"
#include "Maths/Vector3.h"

namespace Insight::Physics
{
    class PhysicsWorld_Jolt;
    class PhysicsWorld_Physx;

    /// Class that constructs a BoxShape
    class IS_PHYSICS BoxShape : public ConvexShape
    {
    public:
        /// Default constructor for deserialization
        BoxShape() = default;

        /// Create a box with half edge length inHalfExtent and convex radius inConvexRadius.
        /// (internally the convex radius will be subtracted from the half extent so the total box will not grow with the convex radius).
        BoxShape(const Maths::Vector3 halfExtent, const float convexRadius = cDefaultConvexRadius);

        Maths::Vector3 HalfExtent = Maths::Vector3::Zero;								///< Half the size of the box (including convex radius)
        float ConvexRadius = 0.0f;
    };
}