#pragma once

#include "Maths/Defines.h"

namespace Insight::Maths
{
	IS_MATHS constexpr float DegreesToRadians(const float degrees)
	{
		return degrees * static_cast<float>(0.01745329251994329576923690768489);
	}

	IS_MATHS constexpr float RadiansToDegrees(const float radians)
	{
		return radians * static_cast<float>(57.295779513082320876798154814105);
	}

	IS_MATHS constexpr float Lerp(const float a, const float b, const float f)
	{
		return a * (1.0f - f) + (b * f);
	}
}