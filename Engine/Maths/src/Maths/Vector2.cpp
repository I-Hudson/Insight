#include "Maths/Vector2.h"
#include "Maths/Matrix2.h"

#include "Maths/MathsUtils.h"

#include <limits>

namespace Insight
{
	namespace Maths
	{
		const Vector2 Vector2::One			= Vector2(1.0f);
		const Vector2 Vector2::Zero			= Vector2(0.0f);
		const Vector2 Vector2::Infinity		= Vector2(std::numeric_limits<float>::infinity());
		const Vector2 Vector2::InfinityNeg	= Vector2(-std::numeric_limits<float>::infinity());

		Vector2::Vector2()
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorReplicate(0.0f))
#else
			: x(0.0f), y(0.0f)
#endif
		{ }
		Vector2::Vector2(float x, float y)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorSet(x, y, 0.0f, 0.0f))
#else
			: x(x), y(y)
#endif
		{ }
		Vector2::Vector2(float value)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorSet(value, value, 0.0f, 0.0f))
#else
			: x(value), y(value)
#endif
		{ }

		Vector2::Vector2(const Vector2& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(other.xmvector)
#else
			: x(other.x), y(other.y)
#endif
		{ }
		Vector2::Vector2(Vector2&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(other.xmvector)
#else
			: x(other.x), y(other.y)
#endif
		{
			other = 0.0f;
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Vector2::Vector2(const DirectX::XMVECTOR& other)
			: xmvector(other)
		{ }
		Vector2::Vector2(DirectX::XMVECTOR&& other)
			: xmvector(other)
		{
			other = DirectX::XMVectorReplicate(0.0f);
		}
#endif

		Vector2::~Vector2()
		{ }

		float Vector2::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Length(xmvector));
#else
			return static_cast<float>(sqrt(LengthSquared()));
#endif
		}
		float Vector2::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2LengthSq(xmvector));
#else
			return (x * x) + (y * y);
#endif
		}

		void Vector2::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVector2Normalize(xmvector);
#else
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
			}
#endif
		}
		Vector2 Vector2::Normalised() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVector2Normalize(xmvector));
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

		float Vector2::Dot(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector2Dot(xmvector, other.xmvector));
#else
			return (x * other.x) + (y * other.y);
#endif
		}

		float& Vector2::operator[](int i)
		{
			return data[i];
		}

		float& Vector2::operator[](unsigned int i)
		{
			return data[i];
		}

		bool Vector2::operator==(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#else
			return Equals(x, other.x) && Equals(y, other.y);
#endif
		}
		bool Vector2::operator!=(const Vector2& other) const
		{
			return !(*this == other);
		}

		Vector2 Vector2::operator=(float value)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorReplicate(value);
#else
			x = value;
			y = value;
#endif
			return *this;
		}
		Vector2 Vector2::operator=(const Vector2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = other.xmvector;
#else
			x = other.x;
			y = other.y;
#endif
			return *this;
		}

		Vector2 Vector2::operator*(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorMultiply(xmvector, DirectX::XMVectorReplicate(scalar)));
#else
			return Vector2(x * scalar, y * scalar);
#endif
		}
		Vector2 Vector2::operator*(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#else
			return Vector2(x * other.x, y * other.y);
#endif
		}

		Vector2 Vector2::operator/(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorReplicate(scalar)));
#else
			return Vector2(x / scalar, y / scalar);
#endif
		}
		Vector2 Vector2::operator/(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorDivide(xmvector, other.xmvector));
#else
			return Vector2(x / other.x, y / other.y);
#endif
		}

		Vector2 Vector2::operator+(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorReplicate(scalar)));
#else
			return Vector2(x + scalar, y + scalar);
#endif
		}
		Vector2 Vector2::operator+(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorAdd(xmvector, other.xmvector));
#else
			return Vector2(x + other.x, y + other.y);
#endif
		}

		Vector2 Vector2::operator-(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorReplicate(scalar)));

#else
			return Vector2(x - scalar, y - scalar);
#endif
		}
		Vector2 Vector2::operator-(const Vector2& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#else
			return Vector2(x - other.x, y - other.y);
#endif
		}

		Vector2 Vector2::operator*=(float scalar)
		{
			*this = Vector2(*this) * scalar;
			return *this;
		}
		Vector2 Vector2::operator*=(const Vector2& other)
		{
			*this = Vector2(*this) * other;
			return *this;
		}

		Vector2 Vector2::operator/=(float scalar)
		{
			*this = Vector2(*this) / scalar;
			return *this;
		}
		Vector2 Vector2::operator/=(const Vector2& other)
		{
			*this = Vector2(*this) / other;
			return *this;
		}

		Vector2 Vector2::operator+=(float scalar)
		{
			*this = Vector2(*this) + scalar;
			return *this;
		}
		Vector2 Vector2::operator+=(const Vector2& other)
		{
			*this = Vector2(*this) + other;
			return *this;
		}

		Vector2 Vector2::operator-=(float scalar)
		{
			*this = Vector2(*this) - scalar;
			return *this;
		}
		Vector2 Vector2::operator-=(const Vector2& other)
		{
			*this = Vector2(*this) - other;
			return *this;
		}
	}
}

#ifdef IS_TESTING
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector2")
	{
		TEST_CASE("Constructors")
		{
			Vector2 vec = Vector2();
			CHECK(vec.x == 0.0f);
			CHECK(vec.y == 0.0f);

			vec = Vector2(24.8f, 64.9f);
			CHECK(vec.x == 24.8f);
			CHECK(vec.y == 64.9f);

			vec = Vector2(117.117f);
			CHECK(vec.x == 117.117f);
			CHECK(vec.y == 117.117f);
		}

		TEST_CASE("Multiplication")
		{
			Vector2 one = Vector2(10.0f, 50.0f);
			Vector2 two = Vector2(5.0f, 2.5f);

			Vector2 resultA = one * two;
			Vector2 resultB = two * one;
			CHECK(resultA.x == 50.0f);
			CHECK(resultA.y == 125.0f);
			CHECK(resultB.x == 50.0f);
			CHECK(resultB.y == 125.0f);

			resultA = one *= Vector2(0.15f, 75.0f);
			CHECK(one.x == 1.5f);
			CHECK(one.y == 3750.0f);
			CHECK(resultA.x == one.x);
			CHECK(resultA.y == one.y);
		}

		TEST_CASE("Divide")
		{
			Vector2 one = Vector2(10.0f, 50.0f);
			Vector2 two = Vector2(5.0f, 2.5f);

			Vector2 resultA = one / two;
			Vector2 resultB = two / one;

			CHECK(resultA.x == 2.0f);
			CHECK(resultA.y == 20.0f);
			CHECK(resultB.x == 0.5f);
			CHECK(resultB.y == 0.05f);

			resultA = one /= Vector2(50.0f, 5.0f);

			CHECK(one.x == 0.2f);
			CHECK(one.y == 10.0f);
			CHECK(resultA.x == one.x);
			CHECK(resultA.y == one.y);
		}

		TEST_CASE("Length")
		{
			Vector2 one = Vector2(10.0f, 5.0f);
			CHECK(Equals(one.Length(), 11.180f, 0.001f));
			CHECK(Equals(one.LengthSquared(), 125.00f, 0.001f));
		}

		TEST_CASE("Dot")
		{
			Vector2 one = Vector2(1.0, 0.0f);
			Vector2 two = Vector2(0.0f, 1.0f);
			CHECK(one.Dot(two) == 0.0f);

			two = Vector2(-1.0f, 0.0f);
			CHECK(one.Dot(two) == -1.0f);

			one = Vector2(-6, 8);
			two = Vector2(5, 12);
			float result = one.Dot(two);
			CHECK(result == 66.0f);
		}

		TEST_CASE("Equal")
		{
			Vector2 one = Vector2(1.0, 0.0f);
			Vector2 two = Vector2(0.0f, 1.0f);
			CHECK(one != two);
			two = one;
			CHECK(one == two);
		}

		TEST_CASE("Normalise")
		{
			Vector2 one = Vector2(50.f, 0.0f);
			CHECK(one.Normalised() == Vector2(1.0f, 0.0));
			one.Normalise();
			CHECK(one == Vector2(1.0f, 0.0));
		}
	}
}
#endif