#pragma once

#include "Core/TypeAlias.h"

namespace Insight::Physics
{
    enum class MotionType : u8
    {
        Static,						///< Non movable
        Kinematic,					///< Movable using velocities only, does not respond to forces
        Dynamic,					///< Responds to forces as a normal physics object
    };
}