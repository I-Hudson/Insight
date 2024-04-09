#pragma once

#include "Maths/Defines.h"

#include "Maths/Vector2.h"
#include "Maths/Vector3.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif 
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
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
			Vector4(float scalar);

			Vector4(const Vector3& other, float w);
			Vector4(Vector3&& other);

			Vector4(const Vector4& other);
			Vector4(Vector4&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector4(const DirectX::XMVECTOR& other);
			Vector4(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
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
			union
			{
				struct { float x, y, z, w; };
				struct { float r, g, b, a; };
				struct { float data[4]; };
#ifdef IS_MATHS_ENABLE_SWIZZLE
				struct { Vector2 xy; float z; float w; };
				struct { float x; Vector2 yz; float w; };
				struct { float x; float y; Vector2 zw;  };
				struct { Vector3 xyz; };
#endif

#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
				struct { glm::vec4 vec4; };
#endif
			};

			static const Vector4 One;
			static const Vector4 Zero;
			static const Vector4 Infinity;
			static const Vector4 InfinityNeg;
		};
	}
}