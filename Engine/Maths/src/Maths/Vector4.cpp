#include "Maths/Vector4.h"
#include "Maths/Vector3.h"

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
		const Vector4 Vector4::One = Vector4(1.0f);
		const Vector4 Vector4::Zero = Vector4(0.0f);
		const Vector4 Vector4::Infinity = Vector4(std::numeric_limits<float>::infinity());
		const Vector4 Vector4::InfinityNeg = Vector4(-std::numeric_limits<float>::infinity());

		Vector4::Vector4()
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#else
			x = 0.0f; 
			y = 0.0f; 
			z = 0.0f;
			w = 0.0f;
#endif
		}

		Vector4::Vector4(float x, float y, float z, float w)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(x, y, z, w);
#else
			(*this).x = x;
			(*this).y = y;
			(*this).z = z;
			(*this).w = w;
#endif
		}
		
		Vector4::Vector4(float scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
			w = scalar;
#endif
		}

		Vector4::Vector4(const Float2& other, float z, float w)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, z);
			xmvector = DirectX::XMVectorSetW(xmvector, w);
#else
			x = other.x;
			y = other.y;
			(*this).z = z;
			(*this).w = w;
#endif
		}
		Vector4::Vector4(Float2&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, 0.0f);
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#else
			x = other.x;
			y = other.y;
			z = 0.0f;
			w = 0.0f;
#endif
		}

		Vector4::Vector4(const Float3& other, float w)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSetW(other.xmvector, w);
#else
			x = other.x;
			y = other.y;
			z = other.z;
			this->w = w;
#endif
		}
		Vector4::Vector4(Float3&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSetW(other.xmvector, 0.0f);
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = 0.0;
#endif
		}

		Vector4::Vector4(const Float4& other)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
		}
		Vector4::Vector4(Float4&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Vector4::Vector4(const DirectX::XMVECTOR& other)
		{ 
			xmvector = other;
		}

		Vector4::Vector4(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		Vector4::Vector4(const glm::vec4& other)
		{
			vec4 = other;
		}
		Vector4::Vector4(glm::vec4&& other)
		{
			vec4 = other;
		}
#endif

		Vector4::~Vector4()
		{ }

		float Vector4::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec4);
#else
			return static_cast<float>(std::sqrt(LengthSquared()));
#endif
		}
		float Vector4::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec4);
#else
			return (x * x) + (y * y) + (z * z) + (w * w);
#endif
		}

		void Vector4::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVector4Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec4 = glm::normalize(vec4);
#else
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
				w *= length_inverted;
			}
#endif
		}
		Vector4 Vector4::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVector4Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec4);
#else
			const auto length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				return (*this) * length_inverted;
			}
			else
			{
				return *this;
			}
#endif
		}

		float Vector4::Dot(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector4Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec4, other.vec4);
#else
			return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
#endif
		}

		float& Vector4::operator[](int i)
		{
			return data[i];
		}
		const float& Vector4::operator[](int i) const
		{
			return data[i];
		}

		bool Vector4::operator==(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVector4Equal(xmvector, other.xmvector);
#elif defined(IS_MATHS_GLM)
			return vec4 == other.vec4;
#else
			return Equal(other, std::numeric_limits<float>::epsilon());
#endif
		}
		bool Vector4::operator!=(const Vector4& other) const
		{
			return !(*this == other);
		}

		bool Vector4::Equal(const Vector4& other, const float errorRange) const
		{
			const bool xResult = Equals(x, other.x, errorRange);
			const bool yResult = Equals(y, other.y, errorRange);
			const bool zResult = Equals(z, other.z, errorRange);
			const bool wResult = Equals(w, other.w, errorRange);
			return xResult
				&& yResult
				&& zResult
				&& wResult;
		}

		bool Vector4::NotEqual(const Vector4& other, const float errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		Vector4 Vector4::Reciprocal() const
		{
			return Vector4(1.0f / x, 1.0f / y, 1.0f / z, 1.0f / w);
		}

		Vector4 Vector4::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec4;
#else
			return Vector4(-x, -y, -z, -w);
#endif
		}

		Vector4 Vector4::operator=(float scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(scalar);
#elif defined(IS_MATHS_GLM)
			vec4 = glm::vec4(scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
			w = scalar;
#endif
			return *this;
		}
		Vector4 Vector4::operator=(const Vector4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec4 = other.vec4;
#else
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
#endif
			return *this;
		}

		Vector4 Vector4::operator*(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 * scalar);
#else
			return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
#endif
		}
		Vector4 Vector4::operator*(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 * other.vec4);
#else
			return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
#endif
		}

		Vector4 Vector4::operator/(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 / scalar);
#else
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
#endif
		}
		Vector4 Vector4::operator/(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 / other.vec4);
#else
			return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
#endif
		}

		Vector4 Vector4::operator+(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 + scalar);
#else
			return Vector4(x + scalar, y + scalar, z + scalar, w + scalar);
#endif
		}
		Vector4 Vector4::operator+(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 + other.vec4);
#else
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
#endif
		}

		Vector4 Vector4::operator-(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorReplicate(scalar)));
#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 - scalar);
#else
			return Vector4(x - scalar, y - scalar, z - scalar, w - scalar);
#endif
		}
		Vector4 Vector4::operator-(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVectorSubtract(xmvector, other.xmvector));
			#elif defined(IS_MATHS_GLM)
			return Vector4(vec4 - other.vec4);
#else
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
#endif
		}

		Vector4 Vector4::operator*=(float scalar)
		{
			*this = Vector4(*this) * scalar;
			return *this;
		}
		Vector4 Vector4::operator*=(const Vector4& other)
		{
			*this = Vector4(*this) * other;
			return *this;
		}

		Vector4 Vector4::operator/=(float scalar)
		{
			*this = Vector4(*this) / scalar;
			return *this;
		}
		Vector4 Vector4::operator/=(const Vector4& other)
		{
			*this = Vector4(*this) / other;
			return *this;
		}

		Vector4 Vector4::operator+=(float scalar)
		{
			*this = Vector4(*this) + scalar;
			return *this;
		}
		Vector4 Vector4::operator+=(const Vector4& other)
		{
			*this = Vector4(*this) + other;
			return *this;
		}

		Vector4 Vector4::operator-=(float scalar)
		{
			*this = Vector4(*this) - scalar;
			return *this;
		}
		Vector4 Vector4::operator-=(const Vector4& other)
		{
			*this = Vector4(*this) - other;
			return *this;
		}
	}
}

#ifdef IS_TESTING
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "Maths/SimpleMath.h"
#include "Maths/SimpleMath.inl"
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector4")
	{
		TEST_CASE("Size Of")
		{
			CHECK(sizeof(Vector4) == 16);
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float z1 = 0.0f;
			float w1 = 0.0f;
			Vector4 vec = Vector4();
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);
			CHECK(vec.w == w1);

			x1 = 24.8f;
			y1 = 64.9f;
			z1 = 256.5f;
			w1 = 8753.0f;
			vec = Vector4(x1, y1, z1, w1);
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);
			CHECK(vec.w == w1);

			x1 = 117.117f;
			vec = Vector4(x1);
			CHECK(vec.x == x1);
			CHECK(vec.y == x1);
			CHECK(vec.z == x1);
			CHECK(vec.w == x1);

#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(x1 == DirectX::XMVectorGetX(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetY(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetZ(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetW(vec.xmvector));
#elif defined(IS_MATHS_GLM)
			CHECK(x1 == vec.vec4.x);
			CHECK(x1 == vec.vec4.y);
			CHECK(x1 == vec.vec4.z);
			CHECK(x1 == vec.vec4.w);
#endif
		}

		TEST_CASE("Length")
		{
			const float x1 = 747;
			const float y1 = 273;
			const float z1 = 273;
			const float w1 = 845;
			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			Vector4 one = Vector4(x1, y1, z1, w1);

			CHECK(Equals(one.Length(), glm::length(glmVec1), 0.001f));
			CHECK(Equals(one.LengthSquared(), glm::length2(glmVec1), 0.001f));
		}

		TEST_CASE("Normalise")
		{
			const float x1 = 9809;
			const float y1 = 8156;
			const float z1 = 4834;
			const float w1 = 6871;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			Vector4 one = Vector4(x1, y1, z1, w1);

			CHECK(one.Normalised() == Vector4(glm::normalize(glmVec1)));
			one.Normalise();
			CHECK(one == glm::normalize(glmVec1));
		}

		TEST_CASE("Dot")
		{
			const float x1 = 0.2f;
			const float y1 = 0.7f;
			const float z1 = 0.4f;
			const float w1 = 0.8f;

			const float x2 = -0.5f;
			const float y2 = 0.1f;
			const float z2 = -0.15f;
			const float w2 = -0.54f;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			glm::vec4 glmVec2 = glm::vec4(x2, y2, z2, w2);
			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);

			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x3 = -1.0f;
			const float y3 = 0.0f;
			const float z3 = 0.0f;
			const float w3 = 0.0f;
			glmVec2 = glm::vec4(-x3, y3, z3, w3);
			two = Vector4(-x3, y3, z3, w3);
			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x4 = -6;
			const float y4 = 8;
			const float z4 = -5;
			const float w4 = -2;

			const float x5 = 5;
			const float y5 = 12;
			const float z5 = 7;
			const float w5 = 18;

			glmVec1 = glm::vec4(x4, y4, z4, w4);
			glmVec2 = glm::vec4(x5, y5, z5, w5);
			one = Vector4(x4, y4, z4, w4);
			two = Vector4(x5, y5, z5, w5);
			float result = one.Dot(two);
			CHECK(Equals(result, glm::dot(glmVec1, glmVec2), 0.001f));
		}

		TEST_CASE("Access Operators")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			const float w1 = 2.0f;

			Vector4 one = Vector4(x1, y1, z1, w1);
			CHECK(Equals(one[0], x1, 0.001f));
			CHECK(Equals(one[1], y1, 0.001f));
			CHECK(Equals(one[2], z1, 0.001f));
			CHECK(Equals(one[3], w1, 0.001f));

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;
			const float w2 = 23457.0f;
			one[0] = x2;
			one[1] = y2;
			one[2] = z2;
			one[3] = w2;
			CHECK(Equals(one[0], x2, 0.001f));
			CHECK(Equals(one[1], y2, 0.001f));
			CHECK(Equals(one[2], z2, 0.001f));
			CHECK(Equals(one[3], w2, 0.001f));
		}

		TEST_CASE("Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			const float w1 = 2.0f;

			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x1, y1, z1, w1);
			CHECK(one == two);
		}

		TEST_CASE("Not Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			const float w1 = 2.0f;

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;
			const float w2 = 23457.0f;

			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);
			CHECK(one != two);
		}

		TEST_CASE("Assign")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			const float w1 = 2.0f;

			Vector4 one = Vector4(x1, y1, z1, w1);
			const float x2 = 20.0f;
			one = x2;
			CHECK(Equals(one.x, x2, 0.001f));
			CHECK(Equals(one.y, x2, 0.001f));
			CHECK(Equals(one.z, x2, 0.001f));
			CHECK(Equals(one.w, x2, 0.001f));

			const float x3 = 55.0f;
			const float y3 = 75.8f;
			const float z3 = -45.0f;
			const float w3 = -18.0f;
			Vector4 two(x3, y3, z3, w3);
			one = two;
			CHECK(one == two);
		}

		TEST_CASE("Multiplication")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;
			const float w1 = 525.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;
			const float w2 = 6153.0f;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			glm::vec4 glmVec2 = glm::vec4(x2, y2, z2, w2);
			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);

			Vector4 resultA = one * two;
			Vector4 resultB = two * one;
			CHECK(resultA == glmVec1 * glmVec2);
			CHECK(resultB == glmVec2 * glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;
			const float w3 = 3.0f;
			glm::vec4 glmVec3 = glmVec1 *= glm::vec4(x3, y3, z3, w3);
			resultA = one *= Vector4(x3, y3, z3, w3);
			CHECK(one == glmVec3);
			CHECK(resultA == one);
		}

		TEST_CASE("Divide")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;
			const float w1 = 525.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;
			const float w2 = 6153.0f;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			glm::vec4 glmVec2 = glm::vec4(x2, y2, z2, w2);
			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);

			Vector4 resultA = one / two;
			Vector4 resultB = two / one;

			CHECK(resultA == glmVec1 / glmVec2);
			CHECK(resultB == glmVec2 / glmVec1);

			const float x3 = 952.0f;
			const float y3 = 111.0f;
			const float z3 = -785.0f;
			const float w3 = -35.0f;
			glm::vec4 glmVec3 = glmVec1 /= glm::vec4(x3, y3, z3, w3);
			resultA = one /= Vector4(x3, y3, z3, w3);
			CHECK(resultA == glmVec3);
		}

		TEST_CASE("Addition")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;
			const float w1 = 525.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;
			const float w2 = 6153.0f;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			glm::vec4 glmVec2 = glm::vec4(x2, y2, z2, w2);
			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);

			Vector4 resultA = one + two;
			Vector4 resultB = two + one;

			CHECK(resultA.Equal(glmVec1 + glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 + glmVec1, 0.001f));

			const float x3 = 952.0f;
			const float y3 = 111.0f;
			const float z3 = -785.0f;
			const float w3 = -35.0f;
			glm::vec4 glmVec3 = glmVec1 += glm::vec4(x3, y3, z3, w3);
			resultA = one += Vector4(x3, y3, z3, w3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Minus")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;
			const float w1 = 525.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;
			const float w2 = 6153.0f;

			glm::vec4 glmVec1 = glm::vec4(x1, y1, z1, w1);
			glm::vec4 glmVec2 = glm::vec4(x2, y2, z2, w2);
			Vector4 one = Vector4(x1, y1, z1, w1);
			Vector4 two = Vector4(x2, y2, z2, w2);

			Vector4 resultA = one - two;
			Vector4 resultB = two - one;

			CHECK(resultA.Equal(glmVec1 - glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 - glmVec1, 0.001f));

			const float x3 = 952.0f;
			const float y3 = 111.0f;
			const float z3 = -785.0f;
			const float w3 = -35.0f;
			glm::vec4 glmVec3 = glmVec1 -= glm::vec4(x3, y3, z3, w3);
			resultA = one -= Vector4(x3, y3, z3, w3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}
	}
}
#endif