#include "Physics/BodyCreationSettings.h"
#include "Physics/Shapes/Shape.h"

namespace Insight::Physics
{
    BodyCreationSettings::BodyCreationSettings(Ref<IShape> shape, const Maths::Vector3& inPosition, const Maths::Quaternion& inRotation, const MotionType inMotionType, const ObjectLayer inObjectLayer)
        : Shape(shape)
        , m_position(inPosition)
        , m_rotation(inRotation)
        , m_motionType(inMotionType)
        , m_objectLayer(inObjectLayer)
    { }
}