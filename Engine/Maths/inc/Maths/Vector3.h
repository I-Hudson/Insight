#pragma once

#include "Maths/Defines.h"
#include "Maths/Vectors/Float2.h"
#include "Maths/Vectors/Float3.h"
#include "Maths/Vectors/Float4.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif

#include <Reflect/Core/Defines.h>

namespace Insight
{
	namespace Maths
	{
		class Vector4;

		REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
		class IS_MATHS Vector3 : public Float3
		{
		public:
			Vector3();
			Vector3(float x, float y, float z);
			Vector3(float scalar);

			Vector3(const Float2& other, float z);
			Vector3(Float2&& other);

			Vector3(const Float3& other);
			Vector3(Float3&& other);

			Vector3(const Float4& other);
			Vector3(Float4&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector3(const DirectX::XMVECTOR& other);
			Vector3(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Vector3(const glm::vec3& other);
			Vector3(glm::vec3&& other);
#endif

			~Vector3();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector3 Normalised() const;

			float Dot(const Vector3& other) const;
			Vector3 Cross(const Vector3& other) const;

			float& operator[](int i);
			const float& operator[](int i) const;

			bool operator==(const Vector3& other) const;
			bool operator!=(const Vector3& other) const;

			bool Equal(const Vector3& other, const float errorRange) const;
			bool NotEqual(const Vector3& other, const float errorRange) const;

			Vector3 operator-() const;

			Vector3 operator=(float scalar);
			Vector3 operator=(const Vector3& other);

			Vector3 operator*(float scalar) const;
			Vector3 operator*(const Vector3& other) const;

			Vector3 operator/(float scalar) const;
			Vector3 operator/(const Vector3& other) const;

			Vector3 operator+(float scalar) const;
			Vector3 operator+(const Vector3& other) const;

			Vector3 operator-(float scalar) const;
			Vector3 operator-(const Vector3& other) const;

			Vector3 operator*=(float scalar);
			Vector3 operator*=(const Vector3& other);

			Vector3 operator/=(float scalar);
			Vector3 operator/=(const Vector3& other);

			Vector3 operator+=(float scalar);
			Vector3 operator+=(const Vector3& other);

			Vector3 operator-=(float scalar);
			Vector3 operator-=(const Vector3& other);

			const float* Data() const { return &x; }

			static const Vector3 One;
			static const Vector3 Zero;
			static const Vector3 Infinity;
			static const Vector3 InfinityNeg;
		};

		IS_MATHS float Vector3Distance(const Vector3& a, const Vector3& b);
	}
}