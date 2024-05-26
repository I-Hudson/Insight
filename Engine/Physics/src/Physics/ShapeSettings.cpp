#include "Physics/ShapeSettings.h"

namespace Insight::Physics
{
    //=========================
    // ShapeSettings
    //=========================
    ShapeSettings::ShapeSettings(const ShapeTypes shapeType, const ShapeSubTypes shapeSubType)
        : ShapeType(shapeType)
        , ShapeSubType(shapeSubType)
    { }

    //=========================
    // ShapeSettings
    //=========================
    ConvexShapeSettings::ConvexShapeSettings(const ShapeSubTypes shapeSubType)
        : ShapeSettings(ShapeTypes::Convex, shapeSubType)
    { }

    //=========================
    // ShapeSettings
    //=========================
    BoxShapeSettings::BoxShapeSettings(const Maths::Vector3 inHalfExtent, const float inConvexRadius)
        : ConvexShapeSettings(ShapeSubTypes::Box), mHalfExtent(inHalfExtent), mConvexRadius(inConvexRadius) 
    { }
}
