#include "ispch.h"
#include "Insight/Maths/Vector3.h"
#include "Insight/Maths/Matrix3.h"

#include "Insight/Log.h"

namespace Insight
{
	namespace Maths
	{
		Vector3::Vector3() : x(0), y(0), z(0)
		{
		}

		Vector3::Vector3(float a_x, float a_y, float a_z) : x(a_x), y(a_y), z(a_z)
		{
		}

		Vector3::Vector3(const Vector3& a_v3) : x(a_v3.x), y(a_v3.y), z(a_v3.z)
		{
		}

		Vector3::~Vector3()
		{

		}

		Vector3::operator float* ()
		{
			return 0;
			//return static_cast<float*>(x);
		}
		Vector3::operator const float* ()
		{
			return 0;
			//return static_cast<const float*>(x);
		}

		void Vector3::Get(float& a_x, float& a_y, float& a_z) const
		{
			a_x = x;
			a_y = y;
			a_z = z;
		}

		void Vector3::Set(const float& a_x, const float& a_y, const float& a_z)
		{
			x = a_x;
			y = a_y;
			z = a_z;
		}

		float& Vector3::operator[](const int& a_index)
		{
			IS_CORE_ASSERT("Vector3: Index was out of range '{0}'.", a_index >= 0 && a_index < 3);
			return i[a_index];
		}

		Vector3& Vector3::operator=(const Vector3& vector)
		{
			x = vector.x;
			y = vector.y;
			z = vector.z;
			return *this;
		}

		Vector3& Vector3::operator=(const Vector3&& vector) noexcept
		{
			x = vector.x;
			y = vector.y;
			z = vector.z;
			return *this;
		}

		bool Vector3::operator ==(const Vector3& a_v3) const
		{
			return (x == a_v3.x && y == a_v3.y, z == a_v3.z);
		}

		bool Vector3::operator !=(const Vector3& a_v3) const
		{
			return (x != a_v3.x || y != a_v3.y || z != a_v3.z);
		}

		const Vector3 Vector3::operator-() const
		{
			return Vector3(x * -1, y * -1, z * -1);
		}

		Vector3 Vector3::operator +(const float a_scalar) const
		{
			return Vector3(x + a_scalar, y + a_scalar, z + a_scalar);
		}

		Vector3 Vector3::operator +(const Vector3& a_v3) const
		{
			return Vector3(x + a_v3.x, y + a_v3.y, z + a_v3.z);
		}

		Vector3& Vector3::operator +=(const float a_scalar)
		{
			x += a_scalar;
			y += a_scalar;
			z += a_scalar;
			return *this;
		}

		Vector3& Vector3::operator +=(const Vector3& a_v3)
		{
			x += a_v3.x;
			y += a_v3.y;
			z += a_v3.z;
			return *this;
		}

		Vector3 Vector3::operator -(const float a_scalar) const
		{
			return Vector3(x - a_scalar, y - a_scalar, z - a_scalar);
		}

		Vector3 Vector3::operator-(const Vector3& a_v3) const
		{
			return Vector3(x - a_v3.x, y - a_v3.y, z - a_v3.z);
		}

		Vector3& Vector3::operator -=(const float a_scalar)
		{
			x -= a_scalar;
			y -= a_scalar;
			z -= a_scalar;
			return *this;
		}

		Vector3& Vector3::operator -=(const Vector3& a_v3)
		{
			x -= a_v3.x;
			y -= a_v3.y;
			z -= a_v3.z;
			return *this;
		}

		Vector3 Vector3::operator *(const float a_scalar) const
		{
			return Vector3(x * a_scalar, y * a_scalar, z * a_scalar);
		}

		Vector3 Vector3::operator*(const Vector3& a_v3) const
		{
			return Vector3(x * a_v3.x, y * a_v3.y, z * a_v3.z);
		}

		const Vector3   operator*(const float a_scalar, const Vector3& a_v3)
		{
			return(a_v3 * a_scalar);
		}

		Vector3& Vector3::operator *=(const float a_scalar)
		{
			x *= a_scalar;
			y *= a_scalar;
			z *= a_scalar;
			return *this;
		}
		Vector3& Vector3::operator *=(const Vector3& a_v3)
		{
			x *= a_v3.x;
			y *= a_v3.y;
			z *= a_v3.z;
			return *this;
		}

		Vector3 Vector3::operator /(const float a_scalar) const
		{
			return Vector3(x / a_scalar, y / a_scalar, z / a_scalar);
		}

		Vector3 Vector3::operator/(const Vector3& a_v3) const
		{
			return Vector3(x / a_v3.x, y / a_v3.y, z / a_v3.z);
		}

		Vector3& Vector3::operator /=(const float a_scalar)
		{
			x /= a_scalar;
			y /= a_scalar;
			z /= a_scalar;
			return *this;
		}

		Vector3& Vector3::operator /=(const Vector3& a_v3)
		{
			x /= a_v3.x;
			y /= a_v3.y;
			z /= a_v3.z;
			return *this;
		}

		float Vector3::Length() const
		{
			return x * x + y * y + z * z;
		}

		float Vector3::Magnitude() const
		{
			float m = (x * x) + (y * y) + (z * z);
			return sqrtf(m);
		}

		float Vector3::LengthSquared() const
		{
			return (x * x + y * y + z * z) * (x * x + y * y + z * z);
		}

		float Vector3::MagnitudeSquared() const
		{
			return (x * x) * (y * y) * (z * z);
		}

		void Vector3::Truncate(float a_fMaxLength)
		{
			Normalise();

			x *= a_fMaxLength;
			y *= a_fMaxLength;
			z *= a_fMaxLength;
		}

		float  Distance(const Vector3& a_v3A, const Vector3& a_v3B)
		{
			Vector3 distance = a_v3A - a_v3B;
			return distance.Magnitude();
		}

		float DistanceSquared(const Vector3& a_v3A, const Vector3& a_v3B)
		{
			Vector3 distance = a_v3A - a_v3B;
			return distance.MagnitudeSquared();
		}

		float Vector3::Dot(const Vector3& a_v3) const
		{
			return(x * a_v3.x + y * a_v3.y + z * a_v3.z);
		}

		Vector3 Vector3::CrossProduct(const Vector3& a_v3) const
		{
			return Vector3(y * a_v3.z - z * a_v3.y, z * a_v3.x - x * a_v3.z, x * a_v3.y - y * a_v3.x);
		}

		float  Dot(const Vector3& vecA, const Vector3& a_v3B)
		{
			return(vecA.x * a_v3B.x + vecA.y * a_v3B.y + vecA.z * a_v3B.z);
		}

		bool Vector3::IsUnit() const
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

		float* Vector3::Normalise()
		{
			Vector3 vector;
			float length = this->Length();

			if (length != 0)
			{
				vector.x = x / length;
				vector.y = y / length;
				vector.z = z / length;
			}

			return vector;
		}


		const Vector3 Vector3::GetUnit() const
		{
			float m = Magnitude();
			return Vector3(x / m, y / m, z / m);
		}


		Vector3 Vector3::GetPerpendicular(const Vector3& a_v3) const
		{
			return (CrossProduct(a_v3));
		}

		void Vector3::RotateX(float fAngle)
		{
			Matrix3 m3 = Matrix3::M3CreateRotateX(fAngle);
			*this = m3 * *this;
		}

		void Vector3::RotateY(float fAngle)
		{
			Matrix3 m3 = Matrix3::M3CreateRotateY(fAngle);
			*this = m3 * *this;
		}

		void Vector3::RotateZ(float fAngle)
		{
			Matrix3 m3 = Matrix3::M3CreateRotateZ(fAngle);
			*this = m3 * *this;
		}


		void Vector3::Zero()
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}

		void Vector3::One()
		{
			x = 1.0f;
			y = 1.0f;
			z = 1.0f;
		}

		float Vector3::Sum() const
		{
			return (x + y + z);
		}

		float Vector3::Min() const
		{
			if (x < y && x < z)
			{
				return x;
			}
			else if (y < x && y < z)
			{
				return y;
			}
			else
			{
				return z;
			}
		}

		float Vector3::Max() const
		{
			if (x > y&& x > z)
			{
				return x;
			}
			else if (y > x&& y > z)
			{
				return y;
			}
			else
			{
				return z;
			}
		}
	}
}