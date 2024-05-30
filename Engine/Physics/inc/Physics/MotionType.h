#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

namespace Insight::Physics
{
    enum class MotionType : u8
    {
        Static,						///< Non movable
        Kinematic,					///< Movable using velocities only, does not respond to forces
        Dynamic,					///< Responds to forces as a normal physics object

        Size
    };
    constexpr const char* MotionTypeToString[] =
    {
        "Static",
        "Kinematic",
        "Dynamic",
    };
    static_assert(ARRAY_COUNT(MotionTypeToString) == static_cast<u64>(MotionType::Size));
}