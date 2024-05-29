#include "Physics/Shapes/BoxShape.h"

namespace Insight::Physics
{
    BoxShape::BoxShape(const Maths::Vector3 halfExtent, const float convexRadius)
        : ConvexShape(ShapeSubTypes::Box)
        , HalfExtent(halfExtent)
        , ConvexRadius(convexRadius)
    { }
}