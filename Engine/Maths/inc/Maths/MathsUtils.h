#pragma once

/**
 * @file MathsUtils.h
 *
 * Util file for any maths functions.
 *
 */

#include "Maths/Defines.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif

#include <cmath>
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
        constexpr float PI_1_DIV_2 = 0.159154943f;
        constexpr float PI_DIV_2 = 1.57079632679f;
        constexpr float PI_DIV_4 = 0.78539816339f;
        constexpr float PI_INV = 0.31830988618f;
        constexpr float DEG_TO_RAD = PI / 180.0f;
        constexpr float RAD_TO_DEG = 180.0f / PI;

        enum class MathsLibrary
        {
            Insight,
            DirectXMath,
            GLM
        };

        /// @brief Return an enum value of type 'MathsLibrary' with the enabled maths library in use. 
        /// @return MathsLibrary
        IS_MATHS constexpr MathsLibrary EnabledMathsLibrary();

        /// @brief Check for equality but allow for a small error
        /// @return bool
        template <class T>
        constexpr bool Equals(T lhs, T rhs, T error = std::numeric_limits<T>::epsilon()) { return (std::abs(lhs - rhs) <= error); }

        //IS_MATHS Matrix4 AxisAngleMatrix(Vector4 vec, float angle);
        //IS_MATHS Matrix4 Matrix4RotationNormal(Vector4 normalAxis, float Angle);

        void ScalerSinCos(float* pSin, float* pCos, float  Value);
	}
}