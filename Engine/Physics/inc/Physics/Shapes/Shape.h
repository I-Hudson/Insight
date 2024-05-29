#pragma once

#include "Physics/Defines.h"
#include "Physics/Shapes/ShapeTypes.h"

#include "Core/RefCount.h"

namespace Insight::Physics
{
    class IS_PHYSICS IShape : public Core::RefCount
    {
    public:
        IShape(const ShapeTypes shapeType, const ShapeSubTypes shapeSubType);

        ShapeTypes ShapeType;
        ShapeSubTypes ShapeSubType;
    };

    /// Class that constructs a ConvexShape (abstract)
    class IS_PHYSICS ConvexShape: public IShape
    {
    public:
        /// Constructor
        ConvexShape(const ShapeSubTypes shapeSubType);

        /// Set the density of the object in kg / m^3
        void							SetDensity(float inDensity) { mDensity = inDensity; }

        // Properties
        //RefConst<PhysicsMaterial>		mMaterial;													///< Material assigned to this shape
        float							mDensity = 1000.0f;											///< Uniform density of the interior of the convex object (kg / m^3)
    };
}