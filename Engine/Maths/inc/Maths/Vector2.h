#pragma once

#include "Maths/Defines.h"
#include "Maths/Vectors/Float2.h"

#include <Reflect/Core/Defines.h>

namespace Insight
{
	namespace Maths
	{
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
		template<typename T>
		class Vec<2, T>
		{
		public:
			Vec();
			
			Vec(const T x, const T y);
			Vec(const T value);

			Vec(const Vec<2, T>& other);
			Vec(Vec<2, T>&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vec(const DirectX::XMVECTOR& other);
			Vec(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Vec(const glm::vec2& other);
			Vec(glm::vec2&& other);
#endif

			~Vec();

			T Length() const;
			T LengthSquared() const;

			Vec<2, T>& Normalise();
			Vec<2, T> Normalised() const;

			T Dot(const Vec<2, T>& other) const;

			T& operator[](int i);
			const T& operator[](int i) const;

			bool operator==(const Vec<2, T>& other) const;
			bool operator!=(const Vec<2, T>& other) const;

			bool Equal(const Vec<2, T>& other, const T errorRange) const;
			bool NotEqual(const Vec<2, T>& other, const T errorRange) const;

			Vec<2, T> operator-() const;

			Vec<2, T>& operator=(T scalar);
			Vec<2, T>& operator=(const Vec<2, T>& other);

			Vec<2, T> operator*(T scalar) const;
			Vec<2, T> operator*(const Vec<2, T>& other) const;

			Vec<2, T> operator/(T scalar) const;
			Vec<2, T> operator/(const Vec<2, T>& other) const;

			Vec<2, T> operator+(T scalar) const;
			Vec<2, T> operator+(const Vec<2, T>& other) const;

			Vec<2, T> operator-(T scalar) const;
			Vec<2, T> operator-(const Vec<2, T>& other) const;

			Vec<2, T>& operator*=(T scalar);
			Vec<2, T>& operator*=(const Vec<2, T>& other);

			Vec<2, T>& operator/=(T scalar);
			Vec<2, T>& operator/=(const Vec<2, T>& other);

			Vec<2, T>& operator+=(T scalar);
			Vec<2, T>& operator+=(const Vec<2, T>& other);

			Vec<2, T>& operator-=(T scalar);
			Vec<2, T>& operator-=(const Vec<2, T>& other);

			const T* Data() const { return &x; }

			static const Vec<2, T> One;
			static const Vec<2, T> Zero;
			static const Vec<2, T> Infinity;
			static const Vec<2, T> InfinityNeg;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
				struct { glm::vec<2, T, glm::defaultp> vec2; };
#endif
				struct { T x, y; };
				struct { T r, g; };
				struct { T data[2]; };
			};
		};

		using Vector2 = Vec<2, float>;
		using DVector2 = Vec<2, double>;
		using IVector2 = Vec<2, int>;
	}
}

#include "Maths/Vector2.inl"