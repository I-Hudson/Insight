#include "Maths/Vector2.h"
#include "Maths/Matrix2.h"

#include "Maths/MathsUtils.h"

#include <cmath>
#include <limits>

#if defined(IS_MATHS_GLM)
#include <glm/gtx/norm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{
		template<typename T>
		const Vec<2, T> Vec<2, T>::One = Vec<2, T>(1.0f);
		template<typename T>
		const Vec<2, T> Vec<2, T>::Zero = Vec<2, T>(0.0f);
		template<typename T>
		const Vec<2, T> Vec<2, T>::Infinity = Vec<2, T>(std::numeric_limits<T>::infinity());
		template<typename T>
		const Vec<2, T> Vec<2, T>::InfinityNeg = Vec<2, T>(-std::numeric_limits<T>::infinity());

		template<typename T>
		Vec<2, T>::Vec()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(0, 0);
#else
			x = 0.0f;
			y = 0.0f;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(const T x, const T y)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(x, y);
#else
			this->x = x;
			this->y = y;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(const T value)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(value, value, 0.0f, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(value, value);
#else
			x = value;
			y = value;
#endif
		}

		template<typename T>
		Vec<2, T>::Vec(const NumberArray2<T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
		}
		template<typename T>
		Vec<2, T>::Vec(NumberArray2<T>&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		template<typename T>
		Vec<2, T>::Vec(const DirectX::XMVECTOR& other)
		{
			xmvector = other;
		}
		template<typename T>
		Vec<2, T>::Vec(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		template<typename T>
		Vec<2, T>::Vec(const glm::vec2& other)
		{
			vec2 = other;
		}
		template<typename T>
		Vec<2, T>::Vec(glm::vec2&& other)
		{
			vec2 = other;
		}
#endif

		template<typename T>
		Vec<2, T>::~Vec()
		{
		}

		template<typename T>
		T Vec<2, T>::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec2);
#else
			return static_cast<T>(sqrt(LengthSquared()));
#endif
		}
		template<typename T>
		T Vec<2, T>::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec2);
#else
			return (x * x) + (y * y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVector2Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::normalize(vec2);
#else
			const T length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
			}
#endif
			return *this;
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVector2Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec2);
#else
			const auto length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const T length_inverted = 1.0f / Length();
				return (*this) * length_inverted;
			}
			else
			{
				return *this;
			}
#endif
		}

		template<typename T>
		T Vec<2, T>::Dot(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec2, other.vec2);
#else
			return (x * other.x) + (y * other.y);
#endif
		}

		template<typename T>
		T& Vec<2, T>::operator[](int i)
		{
			return data[i];
		}
		template<typename T>
		const T& Vec<2, T>::operator[](int i) const
		{
			return data[i];
		}

		template<typename T>
		bool Vec<2, T>::operator==(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return vec2 == other.vec2;
#else
			return Equal(other, std::numeric_limits<T>::epsilon());
#endif
		}
		template<typename T>
		bool Vec<2, T>::operator!=(const Vec<2, T>& other) const
		{
			return !(*this == other);
		}

		template<typename T>
		bool Vec<2, T>::Equal(const Vec<2, T>& other, const T errorRange) const
		{
			return Equals(x, other.x, errorRange) && Equals(y, other.y, errorRange);
		}

		template<typename T>
		bool Vec<2, T>::NotEqual(const Vec<2, T>& other, const T errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec2;
#else
			return Vec<2, T>(-x, -y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(T value)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(value);
#elif defined(IS_MATHS_GLM)
			vec2 = glm::vec2(value, value);
#else
			x = value;
			y = value;
#endif
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator=(const Vec<2, T>& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec2 = other.vec2;
#else
			x = other.x;
			y = other.y;
#endif
			return *this;
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 * scalar);
#else
			return Vec<2, T>(x * scalar, y * scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator*(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 * other.vec2);
#else
			return Vec<2, T>(x * other.x, y * other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 / scalar);
#else
			return Vec<2, T>(x / scalar, y / scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator/(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 / other.vec2);
#else
			return Vec<2, T>(x / other.x, y / other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 + scalar);
#else
			return Vec<2, T>(x + scalar, y + scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator+(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 + other.vec2);
#else
			return Vec<2, T>(x + other.x, y + other.y);
#endif
		}

		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(T scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 - scalar);
#else
			return Vec<2, T>(x - scalar, y - scalar);
#endif
		}
		template<typename T>
		Vec<2, T> Vec<2, T>::operator-(const Vec<2, T>& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vec<2, T>(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vec<2, T>(vec2 - other.vec2);
#else
			return Vec<2, T>(x - other.x, y - other.y);
#endif
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator*=(T scalar)
		{
			*this = Vec<2, T>(*this)* scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator*=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this)* other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator/=(T scalar)
		{
			*this = Vec<2, T>(*this) / scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator/=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) / other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator+=(T scalar)
		{
			*this = Vec<2, T>(*this) + scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator+=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) + other;
			return *this;
		}

		template<typename T>
		Vec<2, T>& Vec<2, T>::operator-=(T scalar)
		{
			*this = Vec<2, T>(*this) - scalar;
			return *this;
		}
		template<typename T>
		Vec<2, T>& Vec<2, T>::operator-=(const Vec<2, T>& other)
		{
			*this = Vec<2, T>(*this) - other;
			return *this;
		}
	}
}

#ifdef IS_TESTING
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <doctest.h>
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vec<2, T>")
	{
		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Vec<2, T>) == 16);
#else
			CHECK(sizeof(Vec<2, T>) == 8);
#endif
		}

		TEST_CASE("Constructors")
		{
			T x1 = 0.0f;
			T y1 = 0.0f;
			Vec<2, T> vec = Vec<2, T>();
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);

			x1 = 24.8f;
			y1 = 64.9f;
			vec = Vec<2, T>(x1, y1);
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);

			x1 = 117.117f;
			y1 = 117.117f;
			vec = Vec<2, T>(x1);
			CHECK(vec.x == x1);
			CHECK(vec.y == x1);

#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(x1 == DirectX::XMVectorGetX(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetY(vec.xmvector));
#elif defined(IS_MATHS_GLM)
			CHECK(x1 == vec.vec2.x);
			CHECK(x1 == vec.vec2.y);
#endif
		}

		TEST_CASE("Length")
		{
			const T v1 = 747;
			const T v2 = 273;
			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			Vec<2, T> one = Vec<2, T>(v1, v2);

			CHECK(Equals(one.Length(), glm::length(glmVec1), 0.001f));
			CHECK(Equals(one.LengthSquared(), glm::length2(glmVec1), 0.001f));
		}

		TEST_CASE("Normalise")
		{
			const T v1 = 9809;
			const T v2 = 8156;
			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			Vec<2, T> one = Vec<2, T>(v1, v2);

			CHECK(one.Normalised() == Vec<2, T>(glm::normalize(glmVec1)));
			one.Normalise();
			CHECK(one == glm::normalize(glmVec1));
		}

		TEST_CASE("Dot")
		{
			const T v1 = 0.2f;
			const T v2 = 0.7f;
			const T v3 = -0.5f;
			const T v4 = 0.1f;

			glm::vec2 glmVec1 = glm::vec2(v1, v2);
			glm::vec2 glmVec2 = glm::vec2(v3, v4);
			Vec<2, T> one = Vec<2, T>(v1, v2);
			Vec<2, T> two = Vec<2, T>(v3, v4);

			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			glmVec2 = glm::vec2(-1.0f, 0.0f);
			two = Vec<2, T>(-1.0f, 0.0f);
			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const T v5 = -6;
			const T v6 = 8;
			const T v7 = 5;
			const T v8 = 12;

			glmVec1 = glm::vec2(v5, v6);
			glmVec2 = glm::vec2(v7, v8);
			one = Vec<2, T>(v5, v6);
			two = Vec<2, T>(v7, v8);
			T result = one.Dot(two);
			CHECK(Equals(result, glm::dot(glmVec1, glmVec2), 0.001f));
		}

		TEST_CASE("Access Operators")
		{
			Vec<2, T> one = Vec<2, T>(10.0f, 5.0f);
			CHECK(Equals(one[0], 10.0f, 0.001f));
			CHECK(Equals(one[1], 5.0f, 0.001f));

			one[0] = 50.0f;
			one[1] = 100.0f;
			CHECK(Equals(one[0], 50.0f, 0.001f));
			CHECK(Equals(one[1], 100.0f, 0.001f));
		}

		TEST_CASE("Equal")
		{
			Vec<2, T> one = Vec<2, T>(10.0f, 5.0f);
			Vec<2, T> two = Vec<2, T>(10.0f, 5.0f);
			CHECK(one == two);
		}

		TEST_CASE("Not Equal")
		{
			Vec<2, T> one = Vec<2, T>(10.0f, 5.0f);
			Vec<2, T> two = Vec<2, T>(5.0f, 10.0f);
			CHECK(one != two);
		}

		TEST_CASE("Assign")
		{
			Vec<2, T> one = Vec<2, T>(10.0f, 5.0f);
			one = 20.0f;
			CHECK(Equals(one.x, 20.0f, 0.001f));
			CHECK(Equals(one.y, 20.0f, 0.001f));

			Vec<2, T> two(55.0f, 75.8f);
			one = two;
			CHECK(one == two);
		}

		TEST_CASE("Multiplication")
		{
			const T x1 = 6487.0f;
			const T y1 = 2575.0f;
			const T x2 = 2006.0f;
			const T y2 = 9618.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vec<2, T> one = Vec<2, T>(x1, y1);
			Vec<2, T> two = Vec<2, T>(x2, y2);

			Vec<2, T> resultA = one * two;
			Vec<2, T> resultB = two * one;
			CHECK(resultA == glmVec1 * glmVec2);
			CHECK(resultB == glmVec2 * glmVec1);

			const T x3 = 0.15f;
			const T y3 = 75.0f;
			glm::vec2 glmVec3 = glmVec1 *= glm::vec2(x3, y3);
			resultA = one *= Vec<2, T>(x3, y3);
			CHECK(one == glmVec3);
			CHECK(resultA == one);
		}

		TEST_CASE("Divide")
		{
			const T x1 = 969.0f;
			const T y1 = 650.0f;
			const T x2 = 485.0f;
			const T y2 = 233.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vec<2, T> one = Vec<2, T>(x1, y1);
			Vec<2, T> two = Vec<2, T>(x2, y2);

			Vec<2, T> resultA = one / two;
			Vec<2, T> resultB = two / one;

			CHECK(resultA == glmVec1 / glmVec2);
			CHECK(resultB == glmVec2 / glmVec1);

			const T x3 = 952.0f;
			const T y3 = 111.0f;

			glm::vec2 glmVec3 = glmVec1 /= glm::vec2(x3, y3);
			resultA = one /= Vec<2, T>(x3, y3);
			CHECK(resultA == glmVec3);
		}

		TEST_CASE("Addition")
		{
			const T x1 = 969.0f;
			const T y1 = 650.0f;
			const T x2 = 485.0f;
			const T y2 = 233.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vec<2, T> one = Vec<2, T>(x1, y1);
			Vec<2, T> two = Vec<2, T>(x2, y2);

			Vec<2, T> resultA = one + two;
			Vec<2, T> resultB = two + one;

			CHECK(resultA.Equal(glmVec1 + glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 + glmVec1, 0.001f));

			const T x3 = 952.0f;
			const T y3 = 111.0f;
			glm::vec2 glmVec3 = glmVec1 += glm::vec2(x3, y3);
			resultA = one += Vec<2, T>(x3, y3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Minus")
		{
			const T x1 = 522.0f;
			const T y1 = 650.0f;
			const T x2 = 485.0f;
			const T y2 = 744.0f;

			glm::vec2 glmVec1 = glm::vec2(x1, y1);
			glm::vec2 glmVec2 = glm::vec2(x2, y2);
			Vec<2, T> one = Vec<2, T>(x1, y1);
			Vec<2, T> two = Vec<2, T>(x2, y2);

			Vec<2, T> resultA = one - two;
			Vec<2, T> resultB = two - one;

			CHECK(resultA.Equal(glmVec1 - glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 - glmVec1, 0.001f));


			const T x3 = 952.0f;
			const T y3 = 111.0f;
			glm::vec2 glmVec3 = glmVec1 -= glm::vec2(x3, y3);
			resultA = one -= Vec<2, T>(x3, y3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}
	}
}
#endif