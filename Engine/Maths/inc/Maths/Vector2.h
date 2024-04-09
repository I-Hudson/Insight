#pragma once

#include "Maths/Defines.h"
#include "Maths/Vectors/Float2.h"

namespace Insight
{
	namespace Maths
	{
		class IS_MATHS Vector2 : public Float2
		{
		public:
			Vector2();
			Vector2(float x, float y);
			Vector2(float value);

			Vector2(const Float2& other);
			Vector2(Float2&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector2(const DirectX::XMVECTOR& other);
			Vector2(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
			Vector2(const glm::vec2& other);
			Vector2(glm::vec2&& other);
#endif

			~Vector2();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector2 Normalised() const;

			float Dot(const Vector2& other) const;

			float& operator[](int i);
			const float& operator[](int i) const;

			bool operator==(const Vector2& other) const;
			bool operator!=(const Vector2& other) const;

			bool Equal(const Vector2& other, const float errorRange) const;
			bool NotEqual(const Vector2& other, const float errorRange) const;

			Vector2 operator-() const;

			Vector2 operator=(float scalar);
			Vector2 operator=(const Vector2& other);

			Vector2 operator*(float scalar) const;
			Vector2 operator*(const Vector2& other) const;

			Vector2 operator/(float scalar) const;
			Vector2 operator/(const Vector2& other) const;

			Vector2 operator+(float scalar) const;
			Vector2 operator+(const Vector2& other) const;

			Vector2 operator-(float scalar) const;
			Vector2 operator-(const Vector2& other) const;

			Vector2 operator*=(float scalar);
			Vector2 operator*=(const Vector2& other);

			Vector2 operator/=(float scalar);
			Vector2 operator/=(const Vector2& other);

			Vector2 operator+=(float scalar);
			Vector2 operator+=(const Vector2& other);

			Vector2 operator-=(float scalar);
			Vector2 operator-=(const Vector2& other);

			const float* Data() const { return &x; }

			static const Vector2 One;
			static const Vector2 Zero;
			static const Vector2 Infinity;
			static const Vector2 InfinityNeg;
		};
	}
}