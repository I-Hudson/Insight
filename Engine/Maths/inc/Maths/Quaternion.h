#pragma once

#include "Maths/Defines.h"

#include "Maths/Vector3.h"
#include "Maths/Vector4.h"

#include <Reflect/Core/Defines.h>

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

namespace Insight
{
    namespace Maths
    {
        REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
        class IS_MATHS Quaternion
        {
        public:
            Quaternion(const Quaternion& quaternion) = default;
            Quaternion(const Maths::Float3& eulerAngles);
            Quaternion(const float eulerX, const float eulerY, const float eulerZ);
            Quaternion(const float w, const float x, const float y, const float z);
            ~Quaternion();

            float& operator[](const int index);
            const float& operator[](const int index) const;

            Quaternion Inversed() const;
            float Dot(const Quaternion& q) const;

            Quaternion& operator=(const Quaternion& q) = default;
            Quaternion& operator +=(const Quaternion& q);
            Quaternion& operator -=(const Quaternion& q);
            Quaternion& operator *=(const Quaternion& q);

            Quaternion& operator *=(const float s);
            Quaternion& operator /=(const float s);

            Quaternion operator+() const;
            Quaternion operator-() const;

            friend Quaternion operator+(const Quaternion& q, const Quaternion& p);
            friend Quaternion operator-(const Quaternion& q, const Quaternion& p);

            friend Quaternion operator*(const Quaternion& q, const Quaternion& p);

            friend Vector3 operator*(const Quaternion& q, const Vector3& v);
            friend Vector3 operator*(const Vector3& v, const Quaternion& q);

            friend Vector4 operator*(const Quaternion& q, const Vector4& v);
            friend Vector4 operator*(const Vector4& v, const Quaternion& q);

            friend Quaternion operator*(const Quaternion& q, const float s);
            friend Quaternion operator*(const float s, const Quaternion& q);

            friend Quaternion operator/(const Quaternion& q, const float s);

            friend bool operator==(const Quaternion& q1, const Quaternion& q2);
            friend bool operator!=(const Quaternion& q1, const Quaternion& q2);

        private:
            Quaternion Conjugate(const Quaternion& q) const;

        public:
            union 
            {
                struct
                {
                    float data[4];
                };

                struct
                {
                    float w;
                    float x;
                    float y;
                    float z;
                };
            };

        };
    }
}