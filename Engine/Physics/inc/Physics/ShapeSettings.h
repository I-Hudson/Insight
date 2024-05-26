#pragma once
#include "Physics/Defines.h"
#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Maths/Vector3.h"

#include <xutility>

namespace Insight::Physics
{
    /// Shapes are categorized in groups, each shape can return which group it belongs to through its Shape::GetType function.
    enum class ShapeTypes
    {
        Convex,							///< Used by ConvexShape, all shapes that use the generic convex vs convex collision detection system (box, sphere, capsule, tapered capsule, cylinder, triangle)
        Compound,						///< Used by CompoundShape
        Decorated,						///< Used by DecoratedShape
        Mesh,							///< Used by MeshShape
        HeightField,					///< Used by HeightFieldShape
        SoftBody,						///< Used by SoftBodyShape
    };
    /// This enumerates all shape types, each shape can return its type through Shape::GetSubType
    enum class ShapeSubTypes
    {
        // Convex shapes
        Sphere,
        Box,
        Triangle,
        Capsule,
        TaperedCapsule,
        Cylinder,
        ConvexHull,

        // Compound shapes
        StaticCompound,
        MutableCompound,

        // Decorated shapes
        RotatedTranslated,
        Scaled,
        OffsetCenterOfMass,

        // Other shapes
        Mesh,
        HeightField,
        SoftBody,
    };

    // Sets of shape sub types
    static constexpr ShapeSubTypes sAllSubShapeTypes[] = { ShapeSubTypes::Sphere, ShapeSubTypes::Box, ShapeSubTypes::Triangle, ShapeSubTypes::Capsule, ShapeSubTypes::TaperedCapsule, ShapeSubTypes::Cylinder, ShapeSubTypes::ConvexHull, ShapeSubTypes::StaticCompound, ShapeSubTypes::MutableCompound, ShapeSubTypes::RotatedTranslated, ShapeSubTypes::Scaled, ShapeSubTypes::OffsetCenterOfMass, ShapeSubTypes::Mesh, ShapeSubTypes::HeightField, ShapeSubTypes::SoftBody };
    static constexpr ShapeSubTypes sConvexSubShapeTypes[] = { ShapeSubTypes::Sphere, ShapeSubTypes::Box, ShapeSubTypes::Triangle, ShapeSubTypes::Capsule, ShapeSubTypes::TaperedCapsule, ShapeSubTypes::Cylinder, ShapeSubTypes::ConvexHull };
    static constexpr ShapeSubTypes sCompoundSubShapeTypes[] = { ShapeSubTypes::StaticCompound, ShapeSubTypes::MutableCompound };
    static constexpr ShapeSubTypes sDecoratorSubShapeTypes[] = { ShapeSubTypes::RotatedTranslated, ShapeSubTypes::Scaled, ShapeSubTypes::OffsetCenterOfMass };

    /// How many shape types we support
    static constexpr u32 NumSubShapeTypes = u32(std::size(sAllSubShapeTypes));

    /// Names of sub shape types
    static constexpr const char* sSubShapeTypeNames[] = { "Sphere", "Box", "Triangle", "Capsule", "TaperedCapsule", "Cylinder", "ConvexHull", "StaticCompound", "MutableCompound", "RotatedTranslated", "Scaled", "OffsetCenterOfMass", "Mesh", "HeightField", "SoftBody" };
    static_assert(std::size(sSubShapeTypeNames) == NumSubShapeTypes);

    /// If objects are closer than this distance, they are considered to be colliding (used for GJK) (unit: meter)
    constexpr float cDefaultCollisionTolerance = 1.0e-4f;

    /// A factor that determines the accuracy of the penetration depth calculation. If the change of the squared distance is less than tolerance * current_penetration_depth^2 the algorithm will terminate. (unit: dimensionless)
    constexpr float cDefaultPenetrationTolerance = 1.0e-4f; ///< Stop when there's less than 1% change

    /// How much padding to add around objects
    constexpr float cDefaultConvexRadius = 0.05f;

    /// Used by (Tapered)CapsuleShape to determine when supporting face is an edge rather than a point (unit: meter)
    static constexpr float cCapsuleProjectionSlop = 0.02f;

    /// Maximum amount of jobs to allow
    constexpr int cMaxPhysicsJobs = 2048;

    /// Maximum amount of barriers to allow
    constexpr int cMaxPhysicsBarriers = 8;

    class ShapeSettings
    {
    public:
        ShapeSettings(const ShapeTypes shapeType, const ShapeSubTypes shapeSubType);

        ShapeTypes ShapeType;
        ShapeSubTypes ShapeSubType;
    };

    /// Class that constructs a ConvexShape (abstract)
    class ConvexShapeSettings : public ShapeSettings
    {
    public:
        /// Constructor
        ConvexShapeSettings(const ShapeSubTypes shapeSubType);

        /// Set the density of the object in kg / m^3
        void							SetDensity(float inDensity) { mDensity = inDensity; }

        // Properties
        //RefConst<PhysicsMaterial>		mMaterial;													///< Material assigned to this shape
        float							mDensity = 1000.0f;											///< Uniform density of the interior of the convex object (kg / m^3)
    };

    /// Class that constructs a BoxShape
    class IS_PHYSICS BoxShapeSettings final : public ConvexShapeSettings
    {
    public:
        /// Default constructor for deserialization
        BoxShapeSettings() = default;

        /// Create a box with half edge length inHalfExtent and convex radius inConvexRadius.
        /// (internally the convex radius will be subtracted from the half extent so the total box will not grow with the convex radius).
        BoxShapeSettings(const Maths::Vector3 inHalfExtent, const float inConvexRadius = cDefaultConvexRadius);

        Maths::Vector3 mHalfExtent = Maths::Vector3::Zero;								///< Half the size of the box (including convex radius)
        float mConvexRadius = 0.0f;
    };
}