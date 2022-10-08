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
			: x(0.0f), y(0.0f), z(0.0f)
		{ }
		Vector3::Vector3(float x, float y, float z)
			: x(x), y(y), z(z)
		{ }
		Vector3::Vector3(float value)
			: x(value), y(value), z(value)
		{ }

		Vector3::Vector3(const Vector3& other)
			: x(other.x), y(other.y), z(other.z)
		{ }
		Vector3::Vector3(Vector3&& other)
			: x(other.x), y(other.y), z(other.z)
		{
			other = 0;
		}

		Vector3::~Vector3()
		{ }

		float Vector3::Length() const
		{
			return static_cast<float>(sqrt(LengthSquared()));
		}
		float Vector3::LengthSquared() const
		{
			return (x * x) + (y * y) + (z * z);
		}

		void Vector3::Normalise()
		{
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
			}
		}
		Vector3 Vector3::Normalised() const
		{
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
		}

		float Vector3::Dot(const Vector3& other) const
		{
			return (x * other.x) + (y * other.y) + (z * other.z);
		}

		float& Vector3::operator[](int i)
		{
			return data[i];
		}

		float& Vector3::operator[](unsigned int i)
		{
			return data[i];
		}

		bool Vector3::operator==(const Vector3& other) const
		{
			return Equals(x, other.x) && Equals(y, other.y) && Equals(z, other.z);
		}
		bool Vector3::operator!=(const Vector3& other) const
		{
			return !(*this == other);
		}

		Vector3 Vector3::operator=(float value)
		{
			x = value;
			y = value;
			z = value;
			return *this;
		}
		Vector3 Vector3::operator=(const Vector3& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		Vector3 Vector3::operator*(float value) const
		{
			return Vector3(x * value, y * value, z * value);
		}
		Vector3 Vector3::operator*(const Vector3& other) const
		{
			return Vector3(x * other.x, y * other.y, z * other.z);
		}

		Vector3 Vector3::operator/(float value) const
		{
			return Vector3(x / value, y / value, z / value);
		}
		Vector3 Vector3::operator/(const Vector3& other) const
		{
			return Vector3(x / other.x, y / other.y, z / other.z);
		}

		Vector3 Vector3::operator+(float value) const
		{
			return Vector3(x + value, y + value, z + value);
		}
		Vector3 Vector3::operator+(const Vector3& other) const
		{
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		Vector3 Vector3::operator-(float value) const
		{
			return Vector3(x - value, y - value, z - value);
		}
		Vector3 Vector3::operator-(const Vector3& other) const
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		Vector3 Vector3::operator*=(float value)
		{
			*this = Vector3(*this) * value;
			return *this;
		}
		Vector3 Vector3::operator*=(const Vector3& other)
		{
			*this = Vector3(*this) * other;
			return *this;
		}

		Vector3 Vector3::operator/=(float value)
		{
			*this = Vector3(*this) / value;
			return *this;
		}
		Vector3 Vector3::operator/=(const Vector3& other)
		{
			*this = Vector3(*this) / other;
			return *this;
		}

		Vector3 Vector3::operator+=(float value)
		{
			*this = Vector3(*this) + value;
			return *this;
		}
		Vector3 Vector3::operator+=(const Vector3& other)
		{
			*this = Vector3(*this) + other;
			return *this;
		}

		Vector3 Vector3::operator-=(float value)
		{
			*this = Vector3(*this) - value;
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