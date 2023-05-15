#include "Maths/MathsUtils.h"

#include <cassert>

namespace Insight
{
	namespace Maths
	{
		constexpr MathsLibrary EnabledMathsLibrary()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return MathsLibrary::DirectXMath;
#else
			return MathsLibrary::Insight;
#endif
		}

		Matrix4 CreatePerspectiveLH(float fov, float aspect, float near, float far)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix4(DirectX::XMMatrixPerspectiveFovLH(fov, aspect, near, far));
#else
            float    SinFov;
            float    CosFov;
            ScalerSinCos(&SinFov, &CosFov, 0.5f * fov);

            float Height = CosFov / SinFov;
            float Width = Height / aspect;
            float fRange = far / (far - near);

            Matrix4 M;
            M[0][0] = Width;
            M[0][1] = 0.0f;
            M[0][2] = 0.0f;
            M[0][3] = 0.0f;

            M[1][0] = 0.0f;
            M[1][1] = Height;
            M[1][2] = 0.0f;
            M[1][3] = 0.0f;

            M[2][0] = 0.0f;
            M[2][1] = 0.0f;
            M[2][2] = fRange;
            M[2][3] = 1.0f;

            M[3][0] = 0.0f;
            M[3][1] = 0.0f;
            M[3][2] = -fRange * near;
            M[3][3] = 0.0f;
            return M;
#endif
		}

        
        Matrix4 CreatePerspectiveRH(float fov, float aspect, float near, float far)
        {
#ifdef IS_MATHS_DIRECTX_MATHS
            return Matrix4(DirectX::XMMatrixPerspectiveFovRH(fov, aspect, near, far));
#else
            float    SinFov;
            float    CosFov;
            ScalerSinCos(&SinFov, &CosFov, 0.5f * fov);

            float Height = CosFov / SinFov;
            float Width = Height / aspect;
            float fRange = far / (near - far);

            Matrix4 M;
            M[0][0] = Width;
            M[0][1] = 0.0f;
            M[0][2] = 0.0f;
            M[0][3] = 0.0f;

            M[1][0] = 0.0f;
            M[1][1] = Height;
            M[1][2] = 0.0f;
            M[1][3] = 0.0f;

            M[2][0] = 0.0f;
            M[2][1] = 0.0f;
            M[2][2] = fRange;
            M[2][3] = -1.0f;

            M[3][0] = 0.0f;
            M[3][1] = 0.0f;
            M[3][2] = fRange * near;
            M[3][3] = 0.0f;
            return M;
#endif
        }

        IS_MATHS Matrix4 AxisAngleMatrix(Vector4 vec, float angle)
        {
#ifdef IS_MATHS_DIRECTX_MATHS
            return Matrix4(DirectX::XMMatrixRotationAxis(vec.xmvector, angle));
#else
            Vector4 normal = vec.Normalised();;
            return Matrix4RotationNormal(normal, angle);
#endif
        }

        IS_MATHS Matrix4 Matrix4RotationNormal(Vector4 normalAxis, float Angle)
        {
#ifdef IS_MATHS_DIRECTX_MATHS
            return Matrix4(DirectX::XMMatrixRotationNormal(normalAxis.xmvector, Angle));
#else
            float    fSinAngle;
            float    fCosAngle;
            ScalerSinCos(&fSinAngle, &fCosAngle, Angle);

            Vector4 A = Vector4(fSinAngle, fCosAngle, 1.0f - fCosAngle, 0.0f);

            Vector4 C2 = Vector4(A.z);
            Vector4 C1 = Vector4(A.y);
            Vector4 C0 = Vector4(A.a);

            Vector4 N0 = Vector4(normalAxis.y, normalAxis.z, normalAxis.x, normalAxis.w);
            Vector4 N1 = Vector4(normalAxis.z, normalAxis.x, normalAxis.y, normalAxis.w);

            Vector4 V0 = C2 * N0;
            V0 = V0 * N1;

            Vector4 R0 = C2 * normalAxis;
            R0 = (R0 * normalAxis) + C1;

            Vector4 R1 = (C0 * normalAxis) + V0;
            Vector4 R2 = V0 - (C0 * normalAxis);

            V0 = Vector4(R0.x, R0.y, R0.z, A.w);
            Vector4 V1 = Vector4(R1.z, R2.y, R2.z, R1.x);
            Vector4 V2 = Vector4(R1.y, R2.x, R1.y, R2.x);

            Matrix4 M;
            M[0] = Vector4(V0.x, V1.x, V1.y, V0.w);
            M[1] = Vector4(V1.z, V0.y, V1.w, V0.w);
            M[2] = Vector4(V2.x, V2.y, V0.z, V0.w);
            M[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
            return M;
#endif
        }

        void ScalerSinCos(float* pSin, float* pCos, float  Value)
        {
            assert(pSin);
            assert(pCos);

            // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
            float quotient = PI_1_DIV_2 * Value;
            if (Value >= 0.0f)
            {
                quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
            }
            else
            {
                quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
            }
            float y = Value - PI_2 * quotient;

            // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
            float sign;
            if (y > PI_DIV_2)
            {
                y = PI - y;
                sign = -1.0f;
            }
            else if (y < -PI_DIV_2)
            {
                y = -PI - y;
                sign = -1.0f;
            }
            else
            {
                sign = +1.0f;
            }

            float y2 = y * y;

            // 11-degree minimax approximation
            *pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

            // 10-degree minimax approximation
            float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
            *pCos = sign * p;
        }
	}
}