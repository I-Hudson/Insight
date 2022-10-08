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
			: x(0.0f), y(0.0f)
		{ }
		Vector2::Vector2(float x, float y)
			: x(x), y(y)
		{ }
		Vector2::Vector2(float value)
			: x(value), y(value)
		{ }

		Vector2::Vector2(const Vector2& other)
			: x(other.x), y(other.y)
		{ }
		Vector2::Vector2(Vector2&& other)
			: x(other.x), y(other.y)
		{
			other = 0;
		}
		Vector2::~Vector2()
		{ }

		float Vector2::Length() const
		{
			return static_cast<float>(sqrt(LengthSquared()));
		}
		float Vector2::LengthSquared() const
		{
			return (x * x) + (y * y);
		}

		void Vector2::Normalise()
		{
			const float length_squared = LengthSquared();
			if (!(length_squared == 1.0f) && length_squared > 0.0f)
			{
				const float length_inverted = 1.0f / Length();
				x *= length_inverted;
				y *= length_inverted;
			}
		}
		Vector2 Vector2::Normalised() const
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

		float Vector2::Dot(const Vector2& other) const
		{
			return (x * other.x) + (y * other.y);
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
			return Equals(x, other.x) && Equals(y, other.y);
		}
		bool Vector2::operator!=(const Vector2& other) const
		{
			return !(*this == other);
		}

		Vector2 Vector2::operator=(float value)
		{
			x = value;
			y = value;
			return *this;
		}
		Vector2 Vector2::operator=(const Vector2& other)
		{
			x = other.x;
			y = other.y;
			return *this;
		}

		Vector2 Vector2::operator*(float value) const
		{
			return Vector2(x * value, y * value);
		}
		Vector2 Vector2::operator*(const Vector2& other) const
		{
			return Vector2(x * other.x, y * other.y);
		}

		Vector2 Vector2::operator/(float value) const
		{
			return Vector2(x / value, y / value);
		}
		Vector2 Vector2::operator/(const Vector2& other) const
		{
			return Vector2(x / other.x, y / other.y);
		}

		Vector2 Vector2::operator+(float value) const
		{
			return Vector2(x + value, y + value);
		}
		Vector2 Vector2::operator+(const Vector2& other) const
		{
			return Vector2(x + other.x, y + other.y);
		}

		Vector2 Vector2::operator-(float value) const
		{
			return Vector2(x - value, y - value);
		}
		Vector2 Vector2::operator-(const Vector2& other) const
		{
			return Vector2(x - other.x, y - other.y);
		}

		Vector2 Vector2::operator*=(float value)
		{
			*this = Vector2(*this) * value;
			return *this;
		}
		Vector2 Vector2::operator*=(const Vector2& other)
		{
			*this = Vector2(*this) * other;
			return *this;
		}

		Vector2 Vector2::operator/=(float value)
		{
			*this = Vector2(*this) / value;
			return *this;
		}
		Vector2 Vector2::operator/=(const Vector2& other)
		{
			*this = Vector2(*this) / other;
			return *this;
		}

		Vector2 Vector2::operator+=(float value)
		{
			*this = Vector2(*this) + value;
			return *this;
		}
		Vector2 Vector2::operator+=(const Vector2& other)
		{
			*this = Vector2(*this) + other;
			return *this;
		}

		Vector2 Vector2::operator-=(float value)
		{
			*this = Vector2(*this) - value;
			return *this;
		}
		Vector2 Vector2::operator-=(const Vector2& other)
		{
			*this = Vector2(*this) - other;
			return *this;
		}
	}
}

#ifdef TEST_ENABLED
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
}
#endif