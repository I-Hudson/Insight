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
		template<typename T>
		class Vec<4, T>;

		template<typename T>
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY)
		class Vec<3, T>
		{
		public:
			Vec();
			Vec(T x, T y, T z);
			Vec(T scalar);

			Vec(const Vec<2, T>& other, T z);
			Vec(Vec<2, T>&& other);

			Vec(const Vec<3, T>& other);
			Vec(Vec<3, T>&& other);

			Vec(const Vec<4, T>& other);
			Vec(Vec<4, T>&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Vec(const DirectX::XMVECTOR& other);
			Vec(DirectX::XMVECTOR&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Vec(const glm::vec3& other);
			Vec(glm::vec3&& other);
#endif

			~Vec();

			T Length() const;
			T LengthSquared() const;

			Vec<3, T>& Normalise();
			Vec<3, T> Normalised() const;

			T Dot(const Vec<3, T>& other) const;
			Vec<3, T> Cross(const Vec<3, T>& other) const;

			T& operator[](int i);
			const T& operator[](int i) const;

			bool operator==(const Vec<3, T>& other) const;
			bool operator!=(const Vec<3, T>& other) const;

			bool Equal(const Vec<3, T>& other, const T errorRange) const;
			bool NotEqual(const Vec<3, T>& other, const T errorRange) const;

			Vec<3, T> Lerp(const Vec<3, T>& vec, const T time) const;

			Vec<3, T> operator-() const;

			Vec<3, T>& operator=(T scalar);
			Vec<3, T>& operator=(const Vec<3, T>& other);

			Vec<3, T> operator*(T scalar) const;
			Vec<3, T> operator*(const Vec<3, T>& other) const;

			Vec<3, T> operator/(T scalar) const;
			Vec<3, T> operator/(const Vec<3, T>& other) const;

			Vec<3, T> operator+(T scalar) const;
			Vec<3, T> operator+(const Vec<3, T>& other) const;

			//Vec<3, T> operator-(T scalar) const;
			//Vec<3, T> operator-(const Vec<3, T>& other) const;

			//IS_MATHS friend Vec<3, T> operator-(T scalar, const Vec<3, T>& vec);
			//IS_MATHS friend Vec<3, T> operator-(const Vec<3, T>& vec, T scalar);
			//IS_MATHS friend Vec<3, T> operator-(const Vec<3, T>& vec, const Vec<3, T>& vec1);

			Vec<3, T>& operator*=(T scalar);
			Vec<3, T>& operator*=(const Vec<3, T>& other);

			Vec<3, T>& operator/=(T scalar);
			Vec<3, T>& operator/=(const Vec<3, T>& other);

			Vec<3, T>& operator+=(T scalar);
			Vec<3, T>& operator+=(const Vec<3, T>& other);

			Vec<3, T>& operator-=(T scalar);
			Vec<3, T>& operator-=(const Vec<3, T>& other);

			const T* Data() const { return &x; }

			static const Vec<3, T> One;
			static const Vec<3, T> Zero;
			static const Vec<3, T> Infinity;
			static const Vec<3, T> InfinityNeg;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				struct { DirectX::XMVECTOR xmvector; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
				struct { glm::vec<3, T, glm::defaultp> vec3; };
#endif
				struct { T x, y, z; };
				struct { T r, g, b; };
				struct { T data[3]; };
			};
		};

		template<typename T>
		Vec<3, T> operator-(T scalar, const Vec<3, T>& vec);
		template<typename T>
		Vec<3, T> operator-(const Vec<3, T>& vec, T scalar);
		template<typename T>
		Vec<3, T> operator-(const Vec<3, T>& vec, const Vec<3, T>& vec1);

		template<typename T>
		T Vector3Distance(const Vec<3, T>& a, const Vec<3, T>& b);

		using Vector3 = Vec<3, float>;
		using DVector3 = Vec<3, double>;
		using IVector3 = Vec<3, int>;
	}
}

#include "Maths/Vector3.inl"