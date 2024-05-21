#include "Maths/Vector3.h"
#include "Maths/MathsUtils.h"
#include <limits>

#if defined(IS_MATHS_GLM)
#include <glm/gtx/norm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{
		const Vector3 Vector3::One			= Vector3(1.0f);
		const Vector3 Vector3::Zero			= Vector3(0.0f);
		const Vector3 Vector3::Infinity		= Vector3(std::numeric_limits<float>::infinity());
		const Vector3 Vector3::InfinityNeg	= Vector3(-std::numeric_limits<float>::infinity());

		Vector3::Vector3()
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(0, 0, 0);
#else
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
#endif
		}
		
		Vector3::Vector3(float x, float y, float z)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(x, y, z, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(x, y, z);
#else
			this.x = x; 
			this.y = y;
			this.z = z;
#endif
		}
		
		Vector3::Vector3(float scalar)
		{ 
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(scalar, scalar, scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
#endif
		}

		Vector3::Vector3(const Float2& other, float z)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, z);
#else
			x = other.x;
			y = other.y;
			z = z;
#endif
		}
		Vector3::Vector3(Float2&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetZ(xmvector, z);
#else
			x = other.x;
			y = other.y;
			z = 0.0f;
#endif
		}

		Vector3::Vector3(const Float3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
		}
		Vector3::Vector3(Float3&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
		}

		Vector3::Vector3(const Float4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(other.vec4.x, other.vec4.y, other.vec4.z);
#else
			x = other.x;
			y = other.y; 
			z = other.z;
#endif
		}
		
		Vector3::Vector3(Float4&& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
			xmvector = DirectX::XMVectorSetW(xmvector, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(other.vec4.x, other.vec4.y, other.vec4.z);
#else
			x = other.x; 
			y = other.y; 
			z = other.z;
#endif
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Vector3::Vector3(const DirectX::XMVECTOR& other)
		{ 
			xmvector = other;
		}
		Vector3::Vector3(DirectX::XMVECTOR&& other)
		{
			xmvector = other;
		}
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
		Vector3::Vector3(const glm::vec3& other)
		{ 
			vec3 = other;
		}
		Vector3::Vector3(glm::vec3&& other)
		{ 
			vec3 = other;
		}
#endif

		Vector3::~Vector3()
		{ }

		float Vector3::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length(vec3);
#else
			return static_cast<float>(sqrt(LengthSquared()));
#endif
		}
		float Vector3::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::length2(vec3);
#else
			return (x * x) + (y * y) + (z * z);
#endif
		}

		void Vector3::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			 xmvector = DirectX::XMVector3Normalize(xmvector);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::normalize(vec3);
#else
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
			}
#endif
		}
		Vector3 Vector3::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVector3Normalize(xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::normalize(vec3);
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

		float Vector3::Dot(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return glm::dot(vec3, other.vec3);
#else
			return (x * other.x) + (y * other.y) + (z * other.z);
#endif
		}
		Vector3 Vector3::Cross(const Vector3& other) const
		{
			return Vector3(
				y * other.z - other.y * z,
				z * other.x - other.z * x,
				x * other.y - other.x * y);
		}

		float& Vector3::operator[](int i)
		{
			return data[i];
		}
		const float& Vector3::operator[](int i) const
		{
			return data[i];
		}

		bool Vector3::operator==(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return vec3 == other.vec3;
#else
			return Equal(other, std::numeric_limits<float>::epsilon());
#endif
		}
		bool Vector3::operator!=(const Vector3& other) const
		{
			return !(*this == other);
		}

		bool Vector3::Equal(const Vector3& other, const float errorRange) const
		{
			return Equals(x, other.x, errorRange) && Equals(y, other.y, errorRange) && Equals(z, other.z, errorRange);
		}

		bool Vector3::NotEqual(const Vector3& other, const float errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		Vector3 Vector3::Lerp(const Vector3& vec, const float time) const
		{
			return Vector3(*this * (1 - time) + (vec * time));
		}

		Vector3 Vector3::operator-() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorNegate(xmvector));
#elif defined(IS_MATHS_GLM)
			return -vec3;
#else
			return Vector3(-x, -y, -z);
#endif
		}

		Vector3 Vector3::operator=(float scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f);
#elif defined(IS_MATHS_GLM)
			vec3 = glm::vec3(scalar);
#else
			x = scalar;
			y = scalar;
			z = scalar;
#endif
			return *this;
		}
		Vector3 Vector3::operator=(const Vector3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#elif defined(IS_MATHS_GLM)
			vec3 = other.vec3;
#else
			x = other.x;
			y = other.y;
			z = other.z;
#endif
			return *this;
		}

		Vector3 Vector3::operator*(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 * scalar);
#else
			return Vector3(x * scalar, y * scalar, z * scalar);
#endif
		}
		Vector3 Vector3::operator*(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 * other.vec3);
#else
			return Vector3(x * other.x, y * other.y, z * other.z);
#endif
		}

		Vector3 Vector3::operator/(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 / scalar);
#else
			return Vector3(x / scalar, y / scalar, z / scalar);
#endif
		}
		Vector3 Vector3::operator/(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorDivide(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 / other.vec3);
#else
			return Vector3(x / other.x, y / other.y, z / other.z);
#endif
		}

		Vector3 Vector3::operator+(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 + scalar);
#else
			return Vector3(x + scalar, y + scalar, z + scalar);
#endif
		}
		Vector3 Vector3::operator+(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorAdd(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 + other.vec3);
#else
			return Vector3(x + other.x, y + other.y, z + other.z);
#endif
		}

		/*
		Vector3 Vector3::operator-(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 - scalar);
#else
			return Vector3(x - scalar, y - scalar, z - scalar);
#endif
		}
		Vector3 Vector3::operator-(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec3 - other.vec3);
#else
			return Vector3(x - other.x, y - other.y, z - other.z);
#endif
		}
		*/

		Vector3 operator-(float scalar, const Vector3& vec)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f), vec.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(scalar - vec);
#else
			return Vector3(scalar - vec.x, scalar - vec.y, scalar - vec.z);
#endif
		}
		Vector3 operator-(const Vector3& vec, float scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(vec.xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec - scalar);
#else
			return Vector3(vec.x - scalar, vec.y - scalar, vec.z - scalar);
#endif
		}
		Vector3 operator-(const Vector3& vec, const Vector3& vec1)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(vec.xmvector, vec1.xmvector));
#elif defined(IS_MATHS_GLM)
			return Vector3(vec.vec3 - vec1.vec3);
#else
			return Vector3(vec.x - otvvec1ec1her.x, vec.y - vec1.y, vec.z - vec1.z);
#endif
		}

		Vector3 Vector3::operator*=(float scalar)
		{
			*this = Vector3(*this) * scalar;
			return *this;
		}
		Vector3 Vector3::operator*=(const Vector3& other)
		{
			*this = Vector3(*this) * other;
			return *this;
		}

		Vector3 Vector3::operator/=(float scalar)
		{
			*this = Vector3(*this) / scalar;
			return *this;
		}
		Vector3 Vector3::operator/=(const Vector3& other)
		{
			*this = Vector3(*this) / other;
			return *this;
		}

		Vector3 Vector3::operator+=(float scalar)
		{
			*this = Vector3(*this) + scalar;
			return *this;
		}
		Vector3 Vector3::operator+=(const Vector3& other)
		{
			*this = Vector3(*this) + other;
			return *this;
		}

		Vector3 Vector3::operator-=(float scalar)
		{
			*this = Vector3(*this) - scalar;
			return *this;
		}
		Vector3 Vector3::operator-=(const Vector3& other)
		{
			*this = Vector3(*this) - other;
			return *this;
		}
	
		float Vector3Distance(const Vector3& a, const Vector3& b)
		{
			return (a - b).Length();
		}
	}
}

#ifdef IS_TESTING
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector3")
	{
		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Vector3) == 16);
#else
			CHECK(sizeof(Vector3) == 12);
#endif
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float z1 = 0.0f;
			Vector3 vec = Vector3();
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);

			x1 = 24.8f;
			y1 = 64.9f;
			z1 = 256.5f;
			vec = Vector3(x1, y1, z1);
			CHECK(vec.x == x1);
			CHECK(vec.y == y1);
			CHECK(vec.z == z1);

			x1 = 117.117f;
			vec = Vector3(x1);
			CHECK(vec.x == x1);
			CHECK(vec.y == x1);
			CHECK(vec.z == x1);

#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(x1 == DirectX::XMVectorGetX(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetY(vec.xmvector));
			CHECK(x1 == DirectX::XMVectorGetZ(vec.xmvector));
#elif defined(IS_MATHS_GLM)
			CHECK(x1 == vec.vec3.x);
			CHECK(x1 == vec.vec3.y);
			CHECK(x1 == vec.vec3.z);
#endif
		}

		TEST_CASE("Length")
		{
			const float x1 = 747;
			const float y1 = 273;
			const float z1 = 273;
			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			Vector3 one = Vector3(x1, y1, z1);

			CHECK(Equals(one.Length(), glm::length(glmVec1), 0.001f));
			CHECK(Equals(one.LengthSquared(), glm::length2(glmVec1), 0.001f));
		}

		TEST_CASE("Normalise")
		{
			const float x1 = 9809;
			const float y1 = 8156;
			const float z1 = 4834;
			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			Vector3 one = Vector3(x1, y1, z1);

			CHECK(one.Normalised() == Vector3(glm::normalize(glmVec1)));
			one.Normalise();
			CHECK(one == glm::normalize(glmVec1));
		}

		TEST_CASE("Dot")
		{
			const float x1 = 0.2f;
			const float y1 = 0.7f;
			const float z1 = 0.4f;
			
			const float x2 = -0.5f;
			const float y2 = 0.1f;
			const float z2 = -0.15f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x3 = -1.0f;
			const float y3 = 0.0f;
			const float z3 = 0.0f;
			glmVec2 = glm::vec3(-x3, y3, z3);
			two = Vector3(-x3, y3, z3);
			CHECK(Equals(one.Dot(two), glm::dot(glmVec1, glmVec2), 0.001f));

			const float x4 = -6;
			const float y4 = 8;
			const float z4 = -5;

			const float x5 = 5;
			const float y5 = 12;
			const float z5 = 7;

			glmVec1 = glm::vec3(x4, y4, z4);
			glmVec2 = glm::vec3(x5, y5, z5);
			one = Vector3(x4, y4, z4);
			two = Vector3(x5, y5, z5);
			float result = one.Dot(two);
			CHECK(Equals(result, glm::dot(glmVec1, glmVec2), 0.001f));
		}

		TEST_CASE("Access Operators")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			Vector3 one = Vector3(x1, y1, z1);
			CHECK(Equals(one[0], x1, 0.001f));
			CHECK(Equals(one[1], y1, 0.001f));
			CHECK(Equals(one[2], z1, 0.001f));

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;
			one[0] = x2;
			one[1] = y2;
			one[2] = z2;
			CHECK(Equals(one[0], x2, 0.001f));
			CHECK(Equals(one[1], y2, 0.001f));
			CHECK(Equals(one[2], z2, 0.001f));
		}

		TEST_CASE("Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x1, y1, z1);
			CHECK(one == two);
		}

		TEST_CASE("Not Equal")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			const float x2 = 6589.0f;
			const float y2 = -457234.0f;
			const float z2 = 865473.0f;

			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);
			CHECK(one != two);
		}

		TEST_CASE("Assign")
		{
			const float x1 = 10.0f;
			const float y1 = 5.0f;
			const float z1 = -45.0f;

			Vector3 one = Vector3(x1, y1, z1);
			const float x2 = 20.0f;
			one = x2;
			CHECK(Equals(one.x, x2, 0.001f));
			CHECK(Equals(one.y, x2, 0.001f));
			CHECK(Equals(one.z, x2, 0.001f));

			const float x3 = 55.0f;
			const float y3 = 75.8f;
			const float z3 = -45.0f;
			Vector3 two(x3, y3, z3);
			one = two;
			CHECK(one == two);
		}

		TEST_CASE("Multiplication")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one * two;
			Vector3 resultB = two * one;
			CHECK(resultA == glmVec1 * glmVec2);
			CHECK(resultB == glmVec2 * glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;
			glm::vec3 glmVec3 = glmVec1 *= glm::vec3(x3, y3, z3);
			resultA = one *= Vector3(x3, y3, z3);
			CHECK(one == glmVec3);
			CHECK(resultA == one);
		}

		TEST_CASE("Divide")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one / two;
			Vector3 resultB = two / one;

			CHECK(resultA == glmVec1 / glmVec2);
			CHECK(resultB == glmVec2 / glmVec1);

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;

			glm::vec3 glmVec3 = glmVec1 /= glm::vec3(x3, y3, z3);
			resultA = one /= Vector3(x3, y3, z3);
			CHECK(resultA == glmVec3);
		}

		TEST_CASE("Addition")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one + two;
			Vector3 resultB = two + one;

			CHECK(resultA.Equal(glmVec1 + glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 + glmVec1, 0.001f));

			const float x3 = 0.15f;
			const float y3 = 75.0f;
			const float z3 = 14.0f;
			glm::vec3 glmVec3 = glmVec1 += glm::vec3(x3, y3, z3);
			resultA = one += Vector3(x3, y3, z3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Minus")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);
			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			Vector3 resultA = one - two;
			Vector3 resultB = two - one;

			CHECK(resultA.Equal(glmVec1 - glmVec2, 0.001f));
			CHECK(resultB.Equal(glmVec2 - glmVec1, 0.001f));


			const float x3 = 952.0f;
			const float y3 = 111.0f;
			const float z3 = -785.0f;
			glm::vec3 glmVec3 = glmVec1 -= glm::vec3(x3, y3, z3);
			resultA = one -= Vector3(x3, y3, z3);

			CHECK(resultA.Equal(glmVec3, 0.001f));
		}

		TEST_CASE("Lerp")
		{
			const float x1 = 6487.0f;
			const float y1 = 2575.0f;
			const float z1 = 845.0f;

			const float x2 = 2006.0f;
			const float y2 = 9618.0f;
			const float z2 = 7453.0f;

			Vector3 one = Vector3(x1, y1, z1);
			Vector3 two = Vector3(x2, y2, z2);

			glm::vec3 glmVec1 = glm::vec3(x1, y1, z1);
			glm::vec3 glmVec2 = glm::vec3(x2, y2, z2);

			for (size_t i = 0; i < 500; ++i)
			{
				const float f = (1.0f / 500) * i;
				Vector3 lerp = one.Lerp(two, f);
				glm::vec3 glmLerp = glm::mix(glmVec1, glmVec2, f);

				CHECK(lerp.x == glmLerp.x);
				CHECK(lerp.y == glmLerp.y);
				CHECK(lerp.z == glmLerp.z);
			}
		}
	}
}
#endif