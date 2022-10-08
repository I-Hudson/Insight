#pragma once

#include "Maths/Defines.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif 

namespace Insight
{
	namespace Maths
	{
		class Vector3;

		/// @brief Vector 4 maths class.
		class IS_MATHS Vector4
		{
		public:
			Vector4();
			Vector4(float x, float y, float z, float w);
			Vector4(float value);

			Vector4(const Vector3& other, float w);
			Vector4(Vector3&& other);

			Vector4(const Vector4& other);
			Vector4(Vector4&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector4(const DirectX::XMVECTOR& other);
			Vector4(DirectX::XMVECTOR&& other);
#endif

			~Vector4();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector4 Normalised() const;

			float Dot(const Vector4& other) const;

			float& operator[](int i);
			float& operator[](unsigned int i);

			bool operator==(const Vector4& other) const;
			bool operator!=(const Vector4& other) const;

			Vector4 operator=(float value);
			Vector4 operator=(const Vector4& other);

			Vector4 operator*(float value) const;
			Vector4 operator*(const Vector4& other) const;

			Vector4 operator/(float value) const;
			Vector4 operator/(const Vector4& other) const;

			Vector4 operator+(float value) const;
			Vector4 operator+(const Vector4& other) const;

			Vector4 operator-(float value) const;
			Vector4 operator-(const Vector4& other) const;

			Vector4 operator*=(float value);
			Vector4 operator*=(const Vector4& other);
							 
			Vector4 operator/=(float value);
			Vector4 operator/=(const Vector4& other);
							 
			Vector4 operator+=(float value);
			Vector4 operator+=(const Vector4& other);
							 
			Vector4 operator-=(float value);
			Vector4 operator-=(const Vector4& other);

			const float* Data() const { return &x; }
			union
			{
				struct { float x, y, z, w; };
				struct { float r, g, b, a; };
				struct { float data[4]; };
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
			};

			static const Vector4 One;
			static const Vector4 Zero;
			static const Vector4 Infinity;
			static const Vector4 InfinityNeg;
		};
	}
}