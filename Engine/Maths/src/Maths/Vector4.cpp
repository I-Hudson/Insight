#include "Maths/Vector4.h"
#include "Maths/Vector3.h"

#include "Maths/MathsUtils.h"

#include <limits>

namespace Insight
{
	namespace Maths
	{
		const Vector4 Vector4::One = Vector4(1.0f);
		const Vector4 Vector4::Zero = Vector4(0.0f);
		const Vector4 Vector4::Infinity = Vector4(std::numeric_limits<float>::infinity());
		const Vector4 Vector4::InfinityNeg = Vector4(-std::numeric_limits<float>::infinity());

		Vector4::Vector4()
			: x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{ }
		Vector4::Vector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{ }
		Vector4::Vector4(float value)
			: x(value), y(value), z(value), w(value)
		{ }

		Vector4::Vector4(const Vector3& other, float w)
			: x(other.x), y(other.y), z(other.z), w(w)
		{ }
		Vector4::Vector4(Vector3&& other)
			: x(other.x), y(other.y), z(other.z), w(0)
		{
			other = 0;
		}

		Vector4::Vector4(const Vector4& other)
			: x(other.x), y(other.y), z(other.z), w(other.w)
		{ }
		Vector4::Vector4(Vector4&& other)
			: x(other.x), y(other.y), z(other.z), w(other.w)
		{
			other = 0;
		}

		Vector4::~Vector4()
		{ }

		float Vector4::Length() const
		{
			return static_cast<float>(sqrt(LengthSquared()));
		}
		float Vector4::LengthSquared() const
		{
			return (x * x) + (y * y) + (z * z) + (w * w);
		}

		void Vector4::Normalise()
		{
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
				z *= length_inverted;
				w *= length_inverted;
			}
		}
		Vector4 Vector4::Normalised() const
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

		float Vector4::Dot(const Vector4& other) const
		{
			return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
		}

		float& Vector4::operator[](int i)
		{
			return data[i];
		}

		float& Vector4::operator[](unsigned int i)
		{
			return data[i];
		}

		bool Vector4::operator==(const Vector4& other) const
		{
			return Equals(x, other.x) && Equals(y, other.y) && Equals(z, other.z) && Equals(w, other.w);
		}
		bool Vector4::operator!=(const Vector4& other) const
		{
			return !(*this == other);
		}

		Vector4 Vector4::operator=(float value)
		{
			x = value;
			x = value;
			x = value;
			w = value;
			return *this;
		}
		Vector4 Vector4::operator=(const Vector4& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
			return *this;
		}

		Vector4 Vector4::operator*(float value) const
		{
			return Vector4(x * value, y * value, z * value, w * value);
		}
		Vector4 Vector4::operator*(const Vector4& other) const
		{
			return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
		}

		Vector4 Vector4::operator/(float value) const
		{
			return Vector4(x / value, y / value, z / value, w / value);
		}
		Vector4 Vector4::operator/(const Vector4& other) const
		{
			return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
		}

		Vector4 Vector4::operator+(float value) const
		{
			return Vector4(x + value, y + value, z + value, w + value);
		}
		Vector4 Vector4::operator+(const Vector4& other) const
		{
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		Vector4 Vector4::operator-(float value) const
		{
			return Vector4(x - value, y - value, z - value, w - value);
		}
		Vector4 Vector4::operator-(const Vector4& other) const
		{
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}

		Vector4 Vector4::operator*=(float value)
		{
			*this = Vector4(*this) * value;
			return *this;
		}
		Vector4 Vector4::operator*=(const Vector4& other)
		{
			*this = Vector4(*this) * other;
			return *this;
		}

		Vector4 Vector4::operator/=(float value)
		{
			*this = Vector4(*this) / value;
			return *this;
		}
		Vector4 Vector4::operator/=(const Vector4& other)
		{
			*this = Vector4(*this) / other;
			return *this;
		}

		Vector4 Vector4::operator+=(float value)
		{
			*this = Vector4(*this) + value;
			return *this;
		}
		Vector4 Vector4::operator+=(const Vector4& other)
		{
			*this = Vector4(*this) + other;
			return *this;
		}

		Vector4 Vector4::operator-=(float value)
		{
			*this = Vector4(*this) - value;
			return *this;
		}
		Vector4 Vector4::operator-=(const Vector4& other)
		{
			*this = Vector4(*this) - other;
			return *this;
		}
	}
}

#ifdef TEST_ENABLED
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Vector4")
	{
		TEST_CASE("Constructors")
		{
			Vector4 vec4 = Vector4();
			CHECK(vec4.x == 0.0f);
			CHECK(vec4.y == 0.0f);
			CHECK(vec4.z == 0.0f);
			CHECK(vec4.w == 0.0f);

			vec4 = Vector4(24.8f, 64.9f, 7816.f, 8513.45f);
			CHECK(vec4.x == 24.8f);
			CHECK(vec4.y == 64.9f);
			CHECK(vec4.z == 7816.f);
			CHECK(vec4.w == 8513.45f);

			vec4 = Vector3(117.117f);
			CHECK(vec4.x == 117.117f);
			CHECK(vec4.y == 117.117f);
			CHECK(vec4.z == 117.117f);
			CHECK(vec4.w == 0.0f);
		}

		TEST_CASE("Multiplcation")
		{
			Vector4 one = Vector4(10.0f, 50.0f, 100.0f, 75.0f);
			Vector4 two = Vector4(5.0f, 2.5f, 0.5f, 0.1f);
			Vector4 result = one * two;
			CHECK(result.x == 50.0f);
			CHECK(result.y == 125.0f);
			CHECK(result.z == 50.0f);
			CHECK(result.w == 7.5f);

			result = one *= Vector4(0.15f, 75.0f, 548.0f, 2.0f);
			CHECK(one.x == 1.5f);
			CHECK(one.y == 3750.0f);
			CHECK(one.z == 54800.0f);
			CHECK(one.w == 150.0f);

			CHECK(result.x == one.x);
			CHECK(result.y == one.y);
			CHECK(result.z == one.z);
			CHECK(result.w == one.w);
		}

		TEST_CASE("Divide")
		{
			Vector4 one = Vector4(10.0f, 50.0f, 100.0f, 15.0f);
			Vector4 two = Vector4(5.0f, 2.5f, 0.5f, 200.0f);
			Vector4 result = one / two;
			CHECK(result.x == 2.0f);
			CHECK(result.y == 20.0f);
			CHECK(result.z == 200.0f);
			CHECK(result.w == 0.075f);

			result = one /= Vector4(50.0f, 5.0f, 1000.0f, 50.0f);
			CHECK(one.x == 0.2f);
			CHECK(one.y == 10.0f);
			CHECK(one.z == 0.1f);
			CHECK(one.w == 0.3f);

			CHECK(result.x == one.x);
			CHECK(result.y == one.y);
			CHECK(result.z == one.z);
			CHECK(result.w == one.w);
		}
	}
}
#endif