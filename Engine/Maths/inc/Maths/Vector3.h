#pragma once

#include "Maths/Defines.h"
#include "Maths/Vector2.h"

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
		class IS_MATHS Vector3
		{
		public:
			Vector3();
			Vector3(float x, float y, float z);
			Vector3(float scalar);

			Vector3(const Vector3& other);
			Vector3(Vector3&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vector3(const DirectX::XMVECTOR& other);
			Vector3(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
			Vector3(const glm::vec3& other);
			Vector3(glm::vec3&& other);
#endif

			~Vector3();

			float Length() const;
			float LengthSquared() const;

			void Normalise();
			Vector3 Normalised() const;

			float Dot(const Vector3& other) const;

			float& operator[](int i);
			const float& operator[](int i) const;

			bool operator==(const Vector3& other) const;
			bool operator!=(const Vector3& other) const;

			bool Equal(const Vector3& other, const float errorRange) const;
			bool NotEqual(const Vector3& other, const float errorRange) const;

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
			union
			{
				struct { float x, y, z; };
				struct { float r, g, b; };
				struct { float data[3]; };
#ifdef IS_MATHS_ENABLE_SWIZZLE
				struct { Vector2 xy; float z;  };
				struct { float x;  Vector2 yz; };
#endif
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
				struct { glm::vec3 vec3; };
#endif
			};

			static const Vector3 One;
			static const Vector3 Zero;
			static const Vector3 Infinity;
			static const Vector3 InfinityNeg;
		};
	}
}