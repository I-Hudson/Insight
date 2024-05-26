#include "Physics/BodyCreationSettings.h"

namespace Insight::Physics
{
    BodyCreationSettings::BodyCreationSettings(const ShapeSettings* inShapeSettings, const Maths::Vector3& inPosition, const Maths::Quaternion& inRotation, const MotionType inMotionType, const ObjectLayer inObjectLayer)
        : m_shapeSettings(inShapeSettings)
        , m_position(inPosition)
        , m_rotation(inRotation)
        , m_motionType(inMotionType)
        , m_objectLayer(inObjectLayer)
    { }
}