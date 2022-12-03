#include "Maths/Vector3.h"
#include "Maths/MathsUtils.h"

#include <limits>

namespace Insight
{
	namespace Maths
	{
		const Vector3 Vector3::One			= Vector3(1.0f);
		const Vector3 Vector3::Zero			= Vector3(0.0f);
		const Vector3 Vector3::Infinity		= Vector3(std::numeric_limits<float>::infinity());
		const Vector3 Vector3::InfinityNeg	= Vector3(-std::numeric_limits<float>::infinity());

		Vector3::Vector3()
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorReplicate(0.0f))
#else
			: x(0.0f), y(0.0f), z(0.0f)
#endif
		{ }
		Vector3::Vector3(float x, float y, float z)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorSet(x, y, z, 0.0f))
#else
			: x(x), y(y), z(z)
#endif
		{ }
		Vector3::Vector3(float scalar)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(DirectX::XMVectorSet(scalar,scalar, scalar, 0.0f))
#else
			: x(scalar), y(scalar), z(scalar)
#endif
		{ }

		Vector3::Vector3(const Vector3& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(other.xmvector)
#else
			: x(other.x), y(other.y), z(other.z)
#endif
		{ }
		Vector3::Vector3(Vector3&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmvector(other.xmvector)
#else
			: x(other.x), y(other.y), z(other.z)
#endif
		{
			other = 0;
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Vector3::Vector3(const DirectX::XMVECTOR& other)
			: xmvector(other)
		{ }
		Vector3::Vector3(DirectX::XMVECTOR&& other)
			: xmvector(other)
		{
			other = DirectX::XMVectorReplicate(0.0f);
		}
#endif

		Vector3::~Vector3()
		{ }

		float Vector3::Length() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(xmvector));
#else
			return static_cast<float>(sqrt(LengthSquared()));
#endif
		}
		float Vector3::LengthSquared() const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(xmvector));
#else
			return (x * x) + (y * y) + (z * z);
#endif
		}

		void Vector3::Normalise()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			 xmvector = DirectX::XMVector3Normalize(xmvector);
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
#else
			return (x * other.x) + (y * other.y) + (z * other.z);
#endif
		}

		float& Vector3::operator[](int i)
		{
			return data[i];
		}

		float& Vector3::operator[](unsigned int i)
		{
			return data[i];
		}
		const float& Vector3::operator[](int i) const
		{
			return data[i];
		}
		const float& Vector3::operator[](unsigned int i) const
		{
			return data[i];
		}

		bool Vector3::operator==(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVectorGetX(DirectX::XMVectorEqual(xmvector, other.xmvector));
#else
			return Equals(x, other.x) && Equals(y, other.y) && Equals(z, other.z);
#endif
		}
		bool Vector3::operator!=(const Vector3& other) const
		{
			return !(*this == other);
		}

		Vector3 Vector3::operator=(float scalar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmvector = DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f);
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
#else
			return Vector3(x * scalar, y * scalar, z * scalar);
#endif
		}
		Vector3 Vector3::operator*(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorMultiply(xmvector, other.xmvector));
#else
			return Vector3(x * other.x, y * other.y, z * other.z);
#endif
		}

		Vector3 Vector3::operator/(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorDivide(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#else
			return Vector3(x / scalar, y / scalar, z / scalar);
#endif
		}
		Vector3 Vector3::operator/(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorDivide(xmvector, other.xmvector));
#else
			return Vector3(x / other.x, y / other.y, z / other.z);
#endif
		}

		Vector3 Vector3::operator+(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorAdd(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#else
			return Vector3(x + scalar, y + scalar, z + scalar);
#endif
		}
		Vector3 Vector3::operator+(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorAdd(xmvector, other.xmvector));
#else
			return Vector3(x + other.x, y + other.y, z + other.z);
#endif
		}

		Vector3 Vector3::operator-(float scalar) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(xmvector, DirectX::XMVectorSet(scalar, scalar, scalar, 0.0f)));
#else
			return Vector3(x - scalar, y - scalar, z - scalar);
#endif
		}
		Vector3 Vector3::operator-(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVectorSubtract(xmvector, other.xmvector));
#else
			return Vector3(x - other.x, y - other.y, z - other.z);
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
	}
}

#ifdef TEST_ENABLED
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector3")
	{
		TEST_CASE("Constructors")
		{
			Vector3 vec3 = Vector3();
			CHECK(vec3.x == 0.0f);
			CHECK(vec3.y == 0.0f);
			CHECK(vec3.z == 0.0f);

			vec3 = Vector3(24.8f, 64.9f, 7816.f);
			CHECK(vec3.x == 24.8f);
			CHECK(vec3.y == 64.9f);
			CHECK(vec3.z == 7816.f);

			vec3 = Vector3(117.117f);
			CHECK(vec3.x == 117.117f);
			CHECK(vec3.y == 117.117f);
			CHECK(vec3.z == 117.117f);
		}

		TEST_CASE("Multiplcation")
		{
			Vector3 one = Vector3(10.0f, 50.0f, 100.0f);
			Vector3 two = Vector3(5.0f, 2.5f, 0.5f);
			Vector3 result = one * two;
			CHECK(result.x == 50.0f);
			CHECK(result.y == 125.0f);
			CHECK(result.z == 50.0f);

			result = one *= Vector3(0.15f, 75.0f, 548.0f);
			CHECK(one.x == 1.5f);
			CHECK(one.y == 3750.0f);
			CHECK(one.z == 54800.0f);
			
			CHECK(result.x == one.x);
			CHECK(result.y == one.y);
			CHECK(result.z == one.z);
		}

		TEST_CASE("Divide")
		{
			Vector3 one = Vector3(10.0f, 50.0f, 100.0f);
			Vector3 two = Vector3(5.0f, 2.5f, 0.5f);
			Vector3 result = one / two;
			CHECK(result.x == 2.0f);
			CHECK(result.y == 20.0f);
			CHECK(result.z == 200.0f);

			result = one /= Vector3(50.0f, 5.0f, 1000.0f);
			CHECK(one.x == 0.2f);
			CHECK(one.y == 10.0f);
			CHECK(one.z == 0.1f);

			CHECK(result.x == one.x);
			CHECK(result.y == one.y);
			CHECK(result.z == one.z);
		}

		TEST_CASE("Dot")
		{
			Vector3 one = Vector3(1.0, 0.0f, 0.0f);
			Vector3 two = Vector3(0.0f, 1.0f, 0.0f);
			CHECK(one.Dot(two) == 0.0f);
			
			two = Vector3(-1.0f, 0.0f, 0.0f);
			CHECK(one.Dot(two) == -1.0f);
		}
	}
}
#endif