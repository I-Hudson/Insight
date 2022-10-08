#pragma once

#include "Maths/Defines.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif 

namespace Insight
{
	namespace Maths
	{
		class IS_MATHS Vector2
		{
		public:
			Vector2();
			Vector2(float x, float y);
			Vector2(float value);

			Vector2(const Vector2& other);
			Vector2(Vector2&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector2(const DirectX::XMVECTOR& other);
			Vector2(DirectX::XMVECTOR&& other);
#endif

			~Vector2();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector2 Normalised() const;

			float Dot(const Vector2& other) const;

			float& operator[](int i);
			float& operator[](unsigned int i);

			bool operator==(const Vector2& other) const;
			bool operator!=(const Vector2& other) const;

			Vector2 operator=(float value);
			Vector2 operator=(const Vector2& other);

			Vector2 operator*(float value) const;
			Vector2 operator*(const Vector2& other) const;

			Vector2 operator/(float value) const;
			Vector2 operator/(const Vector2& other) const;

			Vector2 operator+(float value) const;
			Vector2 operator+(const Vector2& other) const;

			Vector2 operator-(float value) const;
			Vector2 operator-(const Vector2& other) const;

			Vector2 operator*=(float value);
			Vector2 operator*=(const Vector2& other);

			Vector2 operator/=(float value);
			Vector2 operator/=(const Vector2& other);

			Vector2 operator+=(float value);
			Vector2 operator+=(const Vector2& other);

			Vector2 operator-=(float value);
			Vector2 operator-=(const Vector2& other);

			const float* Data() const { return &x; }
			union
			{
				struct { float x, y; };
				struct { float r, g; };
				struct { float data[2]; };
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
			};

			static const Vector2 One;
			static const Vector2 Zero;
			static const Vector2 Infinity;
			static const Vector2 InfinityNeg;
		};
	}
}