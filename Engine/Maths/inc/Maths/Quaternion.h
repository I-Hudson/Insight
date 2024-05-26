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
            Quaternion() = default;
            Quaternion(const Quaternion& quaternion) = default;
            Quaternion(const Maths::Float3& eulerAngles);
            Quaternion(const float eulerX, const float eulerY, const float eulerZ);
            Quaternion(const float w, const float x, const float y, const float z);
            ~Quaternion();

            float& operator[](const int index);
            const float& operator[](const int index) const;

            Quaternion& Normalise();
            Quaternion Normalised() const { return Quaternion(*this).Normalise(); }

            float Length() const;

            Quaternion Inversed() const;

            float Dot(const Quaternion& q) const;
            Quaternion Slerp(const Quaternion& q, const float time) const;

            Quaternion operator=(const Quaternion& q);

            Quaternion& operator +=(const Quaternion& q);
            Quaternion& operator -=(const Quaternion& q);
            Quaternion& operator *=(const Quaternion& q);

            Quaternion& operator *=(const float s);
            Quaternion& operator /=(const float s);

            Quaternion operator+() const;
            Quaternion operator-() const;

            IS_MATHS friend Quaternion operator+(const Quaternion& q, const Quaternion& p);
            IS_MATHS friend Quaternion operator-(const Quaternion& q, const Quaternion& p);

            IS_MATHS friend Quaternion operator*(const Quaternion& q, const Quaternion& p);

            IS_MATHS friend Vector3 operator*(const Quaternion& q, const Vector3& v);
            IS_MATHS friend Vector3 operator*(const Vector3& v, const Quaternion& q);

            IS_MATHS friend Vector4 operator*(const Quaternion& q, const Vector4& v);
            IS_MATHS friend Vector4 operator*(const Vector4& v, const Quaternion& q);

            IS_MATHS friend Quaternion operator*(const Quaternion& q, const float s);
            IS_MATHS friend Quaternion operator*(const float s, const Quaternion& q);

            IS_MATHS friend Quaternion operator/(const Quaternion& q, const float s);

            IS_MATHS friend bool operator==(const Quaternion& q1, const Quaternion& q2);
            IS_MATHS friend bool operator!=(const Quaternion& q1, const Quaternion& q2);

            static const Quaternion Identity;

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