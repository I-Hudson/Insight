#include "Physics/Shapes/Shape.h"

namespace Insight::Physics
{
    //=============================
    // IShape
    //=============================
    IShape::IShape(const ShapeTypes shapeType, const ShapeSubTypes shapeSubType)
        : ShapeType(shapeType)
        , ShapeSubType(shapeSubType)
    { }

    //=============================
    // ConvexShape
    //=============================
    ConvexShape::ConvexShape(const ShapeSubTypes shapeSubType)
        : IShape(ShapeTypes::Convex, shapeSubType)
    { }
}