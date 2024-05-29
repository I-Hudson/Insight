#pragma once

#include "Core/TypeAlias.h"
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
}
