#include "ispch.h"
#include "Insight/Maths/Vector2.h"

namespace Insight
{
	namespace Maths
	{
		Vector2::Vector2() : x(0), y(0)
		{
		}

		Vector2::Vector2(float a_x, float a_y) : x(a_x), y(a_y)
		{
		}

		Vector2::Vector2(const float* a_i) : x(a_i[0]), y(a_i[1])
		{
		}

		Vector2::Vector2(const Vector2& a_v2) : x(a_v2.x), y(a_v2.y)
		{
		}

		Vector2::~Vector2()
		{

		}

		Vector2::operator float* ()
		{
			return static_cast<float*>(&x);
		}

		Vector2::operator const float* ()
		{
			return static_cast<const float*>(&x);
		}

		void Vector2::Get(float& a_x, float& a_y) const
		{
			a_x = x;
			a_y = y;
		}

		void Vector2::Set(const float& a_x, const float& a_y)
		{
			x = a_x;
			y = a_y;
		}

		Vector2& Vector2::operator=(const Vector2& vector)
		{
			x = vector.x;
			y = vector.y;
			return *this;
		}

		Vector2& Vector2::operator=(const Vector2&& vector) noexcept
		{
			x = vector.x;
			y = vector.y;
			return *this;
		}

		bool Vector2::operator == (const Vector2& a_v2) const
		{
			return (x == a_v2.x && y == a_v2.y);
		}

		bool Vector2::operator != (const Vector2& a_v2) const
		{
			return (x != a_v2.x || y != a_v2.y);
		}

		const Vector2 Vector2::operator-() const
		{
			return Vector2(x * -1, y * -1);
		}

		Vector2 Vector2::operator + (const float a_scalar) const
		{
			return Vector2(x + a_scalar, y + a_scalar);
		}

		Vector2 Vector2::operator + (const Vector2& a_v2) const
		{
			return Vector2(x + a_v2.x, y + a_v2.y);
		}

		Vector2& Vector2::operator += (const float a_scalar)
		{
			x += a_scalar;
			y += a_scalar;
			return *this;
		}

		Vector2& Vector2::operator += (const Vector2& a_v2)
		{
			x += a_v2.x;
			y += a_v2.y;
			return *this;
		}

		Vector2 Vector2::operator - (const float a_scalar) const
		{
			return Vector2(x - a_scalar, y - a_scalar);
		}

		Vector2 Vector2::operator - (const Vector2& a_v2) const
		{
			return Vector2(x - a_v2.x, y - a_v2.y);
		}

		Vector2& Vector2::operator -= (const float a_scalar)
		{
			x -= a_scalar;
			y -= a_scalar;
			return *this;
		}

		Vector2& Vector2::operator -= (const Vector2& a_v2)
		{
			x -= a_v2.x;
			y -= a_v2.y;
			return *this;
		}

		Vector2 Vector2::operator * (const float a_scalar) const
		{
			return Vector2(x * a_scalar, y * a_scalar);
		}

		Vector2 Vector2::operator * (const Vector2& a_v2) const
		{
			return Vector2(x * a_v2.x, y * a_v2.y);
		}

		const Vector2 operator*(const float a_scalar, const Vector2& a_v2)
		{
			return (a_v2 * a_scalar);
		}

		Vector2& Vector2::operator *= (const float a_scalar)
		{
			x *= a_scalar;
			y *= a_scalar;
			return *this;
		}

		Vector2& Vector2::operator *= (const Vector2& a_v2)
		{
			x *= a_v2.x;
			y *= a_v2.y;
			return *this;
		}

		Vector2 Vector2::operator / (const float a_scalar) const
		{
			return Vector2(x / a_scalar, y / a_scalar);
		}

		Vector2 Vector2::operator / (const Vector2& a_v2) const
		{
			return Vector2(x / a_v2.x, y / a_v2.y);
		}

		Vector2& Vector2::operator /= (const float a_scalar)
		{
			x /= a_scalar;
			y /= a_scalar;
			return *this;
		}

		Vector2& Vector2::operator /= (const Vector2& a_v2)
		{
			x /= a_v2.x;
			y /= a_v2.y;
			return *this;
		}

		float Vector2::Length() const
		{
			return x * x + y * y;
		}

		float Vector2::Magnitude() const
		{
			float m = (x * x) + (y * y);
			return sqrtf(m);
		}

		float Vector2::LengthSquared() const
		{
			return  (x * x + y * y) * (x * x + y * y);
		}

		float Vector2::MagnitudeSquared() const
		{
			return  (x * x) * (y * y);
		}

		//Returns the arc tangent of x, y in the range of [-pi, +pi]
		//One radian is equivalent to 180 / PI degrees.
		//the returned value is give in radians
		float Vector2::Bearing() const
		{
			return atan2f(x, y);
		}

		float Distance(const Vector2& a_v2A, const Vector2& a_v2B)
		{
			Vector2 distance = a_v2A - a_v2B;
			return distance.Magnitude();
		}

		float DistanceSquared(const Vector2& a_v2A, const Vector2& a_v2B)
		{
			return(a_v2A - a_v2B).MagnitudeSquared();
		}

		float Vector2::Dot(const Vector2& a_v2) const
		{
			return (x * a_v2.x + y * a_v2.y);
		}

		float Dot(const Vector2& vecA, const Vector2& a_v2B)
		{
			return (vecA.x * a_v2B.x + vecA.y * a_v2B.y);
		}

		bool Vector2::IsUnit() const
		{
			float m = Magnitude();
			if (m > 1)
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		float* Vector2::Normalise()
		{
			float length = this->Length();

			if (length != 0)
			{
				x = x / length;
				y = y / length;
			}

			return *this;
		}

		const Vector2 Vector2::GetUnit() const
		{
			float m = Magnitude();
			return Vector2(x / m, y / m);
		}

		Vector2 Vector2::GetPerpendicular() const
		{
			Vector2 PVect;
			PVect.x = y;
			PVect.y = -x;
			return PVect;
		}

		void Vector2::Rotate(float fAngle)
		{
			x = (x + sinf(fAngle));
			y = (y + cosf(fAngle));
		}

		void Vector2::Project(float fAngle, float fDistance)
		{
			fAngle;
			fDistance;
		}

		void Vector2::Zero()
		{
			x = 0.f;
			y = 0.f;
		}

		void Vector2::One()
		{
			x = 1.f;
			y = 1.f;
		}

		float Vector2::Sum() const
		{
			return (x + y);
		}

		float Vector2::Min() const
		{
			if (x < y)
			{
				return x;
			}
			else
			{
				return y;
			}
		}

		float Vector2::Max() const
		{
			if (x > y)
			{
				return x;
			}
			else
			{
				return y;
			}
		}
	}
}