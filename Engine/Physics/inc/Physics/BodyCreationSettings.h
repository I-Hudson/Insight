#pragma once
#include "Physics/Defines.h"

#include "Physics/MotionType.h"
#include "Physics/ObjectLayers.h"
#include "Physics/ShapeSettings.h"

#include "Maths/Vector3.h"
#include "Maths/Quaternion.h"

namespace Insight::Physics
{
    /// @brief Setting for the creation of a rigid body.
    class IS_PHYSICS BodyCreationSettings
    {
    public:
        BodyCreationSettings() = default;
        BodyCreationSettings(const ShapeSettings* inShapeSettings, const Maths::Vector3& inPosition, const Maths::Quaternion& inRotation, const MotionType inMotionType, const ObjectLayer inObjectLayer);

        const ShapeSettings* m_shapeSettings;
        Maths::Vector3 m_position;
        Maths::Quaternion m_rotation;
        MotionType m_motionType;
        ObjectLayer m_objectLayer;
    };
}