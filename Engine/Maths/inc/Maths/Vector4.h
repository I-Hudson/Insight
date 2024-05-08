#pragma once

#include "Maths/Defines.h"
#include "Maths/Vectors/Float4.h"
#include "Maths/Vectors/Float3.h"
#include "Maths/Vectors/Float2.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif 

#include <Reflect.h>

namespace Insight
{
	namespace Maths
	{
		class Vector3;

		/// @brief Vector 4 maths class.
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
		class IS_MATHS Vector4 : public Float4
		{
		public:
			Vector4();
			Vector4(float x, float y, float z, float w);
			Vector4(float scalar);

			Vector4(const Float2& other, float z, float w);
			Vector4(Float2&& other);

			Vector4(const Float3& other, float w);
			Vector4(Float3&& other);

			Vector4(const Float4& other);
			Vector4(Float4&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector4(const DirectX::XMVECTOR& other);
			Vector4(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Vector4(const glm::vec4& other);
			Vector4(glm::vec4&& other);
#endif

			~Vector4();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector4 Normalised() const;

			float Dot(const Vector4& other) const;

			float& operator[](int i);
			const float& operator[](int i) const;

			bool operator==(const Vector4& other) const;
			bool operator!=(const Vector4& other) const;

			bool Equal(const Vector4& other, const float errorRange) const;
			bool NotEqual(const Vector4& other, const float errorRange) const;

			Vector4 Reciprocal() const;

			Vector4 operator-() const;

			Vector4 operator=(float scalar);
			Vector4 operator=(const Vector4& other);

			Vector4 operator*(float scalar) const;
			Vector4 operator*(const Vector4& other) const;

			Vector4 operator/(float scalar) const;
			Vector4 operator/(const Vector4& other) const;

			Vector4 operator+(float scalar) const;
			Vector4 operator+(const Vector4& other) const;

			Vector4 operator-(float scalar) const;
			Vector4 operator-(const Vector4& other) const;

			Vector4 operator*=(float scalar);
			Vector4 operator*=(const Vector4& other);
							 
			Vector4 operator/=(float scalar);
			Vector4 operator/=(const Vector4& other);
							 
			Vector4 operator+=(float scalar);
			Vector4 operator+=(const Vector4& other);
							 
			Vector4 operator-=(float scalar);
			Vector4 operator-=(const Vector4& other);

			const float* Data() const { return &x; }

			static const Vector4 One;
			static const Vector4 Zero;
			static const Vector4 Infinity;
			static const Vector4 InfinityNeg;
		};
	}
}