#pragma once

#include <limits>

namespace Insight
{
	namespace Maths
	{
        constexpr float EPSILON = std::numeric_limits<float>::epsilon();
        constexpr float INFINITY_ = std::numeric_limits<float>::infinity();
        constexpr float PI = 3.14159265359f;
        constexpr float PI_2 = 6.28318530718f;
        constexpr float PI_4 = 12.5663706144f;
        constexpr float PI_DIV_2 = 1.57079632679f;
        constexpr float PI_DIV_4 = 0.78539816339f;
        constexpr float PI_INV = 0.31830988618f;
        constexpr float DEG_TO_RAD = PI / 180.0f;
        constexpr float RAD_TO_DEG = 180.0f / PI;

        // Check for equality but allow for a small error
        template <class T>
        constexpr bool Equals(T lhs, T rhs, T error = std::numeric_limits<T>::epsilon()) { return (std::abs(lhs - rhs) <= error); }
	}
}