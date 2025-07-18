#pragma once

#include "Maths/Defines.h"
#include "Maths/Vectors/Float4.h"
#include "Maths/Vectors/Float3.h"
#include "Maths/Vectors/Float2.h"

#include <Reflect/Core/Defines.h>

namespace Insight
{
	namespace Maths
	{
		/// @brief Vector 4 maths class.
		template<typename T>
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY)
		class Vec<4, T>
		{
		public:
			Vec();
			Vec(T x, T y, T z, T w);
			Vec(T scalar);

			Vec(const Vec<2, T>& other, T z, T w);
			Vec(Vec<2, T>&& other);

			Vec(const Vec<3, T>& other, T w);
			Vec(Vec<3, T>&& other);

			Vec(const Vec<4, T>& other);
			Vec(Vec<4, T>&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vec(const DirectX::XMVECTOR& other);
			Vec(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Vec(const glm::vec4& other);
			Vec(glm::vec4&& other);
#endif

			~Vec();

			T Length() const;
			T LengthSquared() const;

			Vec<4, T>& Normalise();
			Vec<4, T> Normalised() const;

			T Dot(const Vec<4, T>& other) const;

			Vec<4, T> Floor() const;

			T& operator[](int i);
			const T& operator[](int i) const;

			bool operator==(const Vec<4, T>& other) const;
			bool operator!=(const Vec<4, T>& other) const;

			bool Equal(const Vec<4, T>& other, const T errorRange) const;
			bool NotEqual(const Vec<4, T>& other, const T errorRange) const;

			Vec<4, T> Reciprocal() const;
			Vec<4, T> Cross(const Vec<4, T> vec) const;

			Vec<4, T> operator-() const;

			Vec<4, T>& operator=(T scalar);
			Vec<4, T>& operator=(const Vec<4, T>& other);

			Vec<4, T> operator*(T scalar) const;
			Vec<4, T> operator*(const Vec<4, T>& other) const;

			Vec<4, T> operator/(T scalar) const;
			Vec<4, T> operator/(const Vec<4, T>& other) const;

			Vec<4, T> operator+(T scalar) const;
			Vec<4, T> operator+(const Vec<4, T>& other) const;

			Vec<4, T> operator-(T scalar) const;
			Vec<4, T> operator-(const Vec<4, T>& other) const;

			Vec<4, T>& operator*=(T scalar);
			Vec<4, T>& operator*=(const Vec<4, T>& other);
							 
			Vec<4, T>& operator/=(T scalar);
			Vec<4, T>& operator/=(const Vec<4, T>& other);
							 
			Vec<4, T>& operator+=(T scalar);
			Vec<4, T>& operator+=(const Vec<4, T>& other);
							 
			Vec<4, T>& operator-=(T scalar);
			Vec<4, T>& operator-=(const Vec<4, T>& other);

			const T* Data() const { return &x; }

			static const Vec<4, T> One;
			static const Vec<4, T> Zero;
			static const Vec<4, T> Infinity;
			static const Vec<4, T> InfinityNeg;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
				struct { glm::vec<4, T, glm::defaultp> vec4; };
#endif
				struct { T x, y, z, w; };
				struct { T r, g, b, a; };
				struct { T data[4]; };
			};
		};

		template<typename T>
		Vec<3, T> Lerp(const Vec<3, T> vec1, const Vec<3, T> vec2, const float f);
		template<typename T>
		Vec<4, T> Lerp(const Vec<4, T> vec1, const Vec<4, T> vec2, const float f);

		template<typename T>
		Vec<4, T> Select(const Vec<4, T> vec1, const Vec<4, T> vec2, const Vec<4, T> control);

		template<typename T>
		Vec<4, T> Min(const Vec<4, T> vec1, const Vec<4, T> vec2);		
		template<typename T>
		Vec<4, T> Max(const Vec<4, T> vec1, const Vec<4, T> vec2);
		
		using Vector4 = Vec<4, float>;
		using DVector4 = Vec<4, double>;
		using IVector4 = Vec<4, int>;
	}
}

#include "Maths/Vector4.inl"