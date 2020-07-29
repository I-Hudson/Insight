#include "ispch.h"
#include "Insight/Maths/Vector4.h"
#include "Insight/Maths/Matrix4.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace Maths
	{
		Vector4::Vector4() : x(0), y(0), z(0), w(0)
		{
		}

		Vector4::Vector4(float a_x, float a_y, float a_z, float a_w) : x(a_x), y(a_y), z(a_z), w(a_w)
		{
		}

		Vector4::Vector4(const Vector4& a_v3) : x(a_v3.x), y(a_v3.y), z(a_v3.y), w(a_v3.w)
		{
		}

		Vector4::~Vector4()
		{

		}

		//Used for compatible: GL When reading in the first float all the floats next to it in the array will be place in the next
		//memory position, this will allow the first float to be cast then grab the memory next to it for the next value in the array
		//e.g: If x is stored in 00002Ef5B4 then y is stored in 00002Ef5B5.
		Vector4::operator float* ()
		{
			return static_cast<float*>(&x);
		}

		//Used for compatible: GL When reading in the first float all the floats next to it in the array will be place in the next
		//memory position, this will allow the first float to be cast then grab the memory next to it for the next value in the array
		//e.g: If x is stored in 00002Ef5B4 then y is stored in 00002Ef5B5.
		Vector4::operator const float* ()
		{
			return static_cast<const float*>(&x);
		}

		//used to retrieve Vector 4 and pass then values though to 4 floats 
		void Vector4::Get(float& a_x, float& a_y, float& a_z, float& a_w) const
		{
			a_x = x;
			a_y = y;
			a_z = z;
			a_w = w;
		}

		//used to set the Vector 4 with 4 floats which are passed though
		void Vector4::Set(const float& a_x, const float& a_y, const float& a_z, const float& a_w)
		{
			x = a_x;
			y = a_y;
			z = a_z;
			w = a_w;
		}

		float& Vector4::operator[](const int& a_index)
		{
			IS_CORE_ASSERT(a_index >= 0 && a_index < 4, "Vector4: Index was out of range '{0}'.");
			return i[a_index];
		}

		Vector4& Vector4::operator=(const Vector4& vector)
		{
			x = vector.x;
			y = vector.y;
			z = vector.z;
			w = vector.w;
			return *this;
		}

		Vector4& Vector4::operator=(const Vector4&& vector) noexcept
		{
			x = vector.x;
			y = vector.y;
			z = vector.z;
			w = vector.w;
			return *this;
		}

		//used to see if Vector 4 is equal to Vector 4
		bool Vector4::operator ==(const Vector4& a_v4) const
		{
			return (x == a_v4.x && y == a_v4.y, z == a_v4.z && w == a_v4.w);
		}

		//used to see if Vector4 is not equal to Vector 4 a_v4
		bool Vector4::operator !=(const Vector4& a_v4) const
		{
			return (x != a_v4.x || y != a_v4.y || z != a_v4.z || w != a_v4.w);
		}

		//used to turn vector4 values to negatives
		const Vector4 Vector4::operator-() const
		{
			return Vector4(x * -1.f, y * -1.f, z * -1.f, w * -1.f);
		}

		//used to add Vector 4 by a_scalar, returns a Vector 4
		Vector4 Vector4::operator +(const float a_scalar) const
		{
			return Vector4(x + a_scalar, y + a_scalar, z + a_scalar, w + a_scalar);
		}

		//used to add Vector 4 to Vector 4 a_v4, returns a Vector 4
		Vector4 Vector4::operator +(const Vector4& a_v4) const
		{
			return Vector4(x + a_v4.x, y + a_v4.y, z + a_v4.z, w + a_v4.w);
		}

		//used to add Vector 4 to a_scalar, does not return a Vector 4
		Vector4& Vector4::operator +=(const float a_scalar)
		{
			x += a_scalar;
			y += a_scalar;
			z += a_scalar;
			w += a_scalar;
			return *this;
		}
		//used to add Vector 4 to Vector 4 a_v4, returns a Vector 4
		Vector4& Vector4::operator +=(const Vector4& a_v4)
		{
			x += a_v4.x;
			y += a_v4.y;
			z += a_v4.z;
			w += a_v4.w;
			return *this;
		}

		//used to subtract Vector 4 by a_scalar, returns a Vector 4 
		Vector4 Vector4::operator -(const float a_scalar) const
		{
			return Vector4(x - a_scalar, y - a_scalar, z - a_scalar, w - a_scalar);
		}

		////used to subtract Vector 4 by Vector 4 a_v4, returns a Vector 4 
		Vector4 Vector4::operator-(const Vector4& a_v4) const
		{
			return Vector4(x - a_v4.x, y - a_v4.y, z - a_v4.z, w - a_v4.w);
		}

		//used to subtract Vector 4 by a_scalar, does not return Vector4
		Vector4& Vector4::operator -=(const float a_scalar)
		{
			x -= a_scalar;
			y -= a_scalar;
			z -= a_scalar;
			w -= a_scalar;
			return *this;
		}

		//used to subtract Vector 4 by Vector 4 a_v4, does not return Vector4
		Vector4& Vector4::operator -=(const Vector4& a_v4)
		{
			x -= a_v4.x;
			y -= a_v4.y;
			z -= a_v4.z;
			w -= a_v4.w;
			return *this;
		}

		//used to multiply Vector 4 by a_scalar, returns Vector4
		Vector4 Vector4::operator *(const float a_scalar) const
		{
			return Vector4(x * a_scalar, y * a_scalar, z * a_scalar, w * a_scalar);
		}

		//used to multiply Vector 4 by Vector 4 a_v4, returns Vector4
		Vector4 Vector4::operator*(const Vector4& a_v4) const
		{
			return Vector4(x * a_v4.x, y * a_v4.y, z * a_v4.z, w * a_v4.w);
		}

		//used to multiply Vector 4 by a_scalar, does not returns Vector4
		Vector4& Vector4::operator *=(const float a_scalar)
		{
			x *= a_scalar;
			y *= a_scalar;
			z *= a_scalar;
			w *= a_scalar;
			return *this;
		}

		//used to multiply Vector 4 by Vector 4 a_v4, does not returns Vector4
		Vector4& Vector4::operator *=(const Vector4& a_v4)
		{
			x *= a_v4.x;
			y *= a_v4.y;
			z *= a_v4.z;
			w *= a_v4.w;
			return *this;
		}

		//used to divide Vector 4 by a_scalar, returns Vector4
		Vector4 Vector4::operator /(const float a_scalar) const
		{
			return Vector4(x / a_scalar, y / a_scalar, z / a_scalar, w / a_scalar);
		}

		//used to divide Vector 4 by Vector 4 a_v4, returns Vector4
		Vector4 Vector4::operator/(const Vector4& a_v4) const
		{
			return Vector4(x / a_v4.x, y / a_v4.y, z / a_v4.z, w / a_v4.w);
		}

		//used to divide Vector 4 by a_scalar, does not return Vector4
		Vector4& Vector4::operator /=(const float a_scalar)
		{
			x /= a_scalar;
			y /= a_scalar;
			z /= a_scalar;
			w /= a_scalar;
			return *this;
		}

		//used to divide Vector 4 by Vector 4 a_v4, does not return Vector4
		Vector4& Vector4::operator /=(const Vector4& a_v4)
		{
			x /= a_v4.x;
			y /= a_v4.y;
			z /= a_v4.z;
			w /= a_v4.w;
			return *this;
		}

		//used to get the length of the Vector 4
		//Length: Distance between two points
		float Vector4::Length() const
		{
			return x * x + y * y + z * z + w * w;

		}

		//used to get the magnitude of the Vector 4
		//Magnitude: Size of a object
		float Vector4::Magnitude() const
		{
			float m = (x * x) + (y * y) + (z * z) + (w * w);
			return sqrtf(m);
		}

		//Get Length Squared
		float Vector4::LengthSquared() const
		{
			return (x * x + y * y + z * z + w * w) * (x * x + y * y + z * z + w * w);
		}

		//get Magnitude Squared
		float Vector4::MagnitudeSquared() const
		{
			return (x * x) * (y * y) * (z * z) * (w * w);
		}

		//Normalises the Vector to a max length which is passed though
		void Vector4::Truncate(float a_fMaxLength)
		{
			Normalise();

			x *= a_fMaxLength;
			y *= a_fMaxLength;
			z *= a_fMaxLength;
			w *= a_fMaxLength;
		}

		//returns a float value which is the distance between the first and last vector2 passed though
		float  Distance(const Vector4& a_v4A, const Vector4& a_v4B)
		{
			Vector4 distance = a_v4A - a_v4B;
			return distance.Magnitude();
		}

		//returns a float value which is the distance between the first and last vector2 passed though, then squares the result
		float DistanceSquared(const Vector4& a_v4A, const Vector4& a_v4B)
		{
			Vector4 distance = a_v4A - a_v4B;
			return distance.MagnitudeSquared();
		}

		//used to get the dot project of the Vector 3 
		float Vector4::Dot(const Vector4& a_v4) const
		{
			return(x * a_v4.x + y * a_v4.y + z * a_v4.z + w * a_v4.w);
		}

		//used to get cross project of the Vector 3
		Vector4 Vector4::CrossProduct(const Vector4& a_v4) const
		{
			return Vector4(y * a_v4.z - z * a_v4.y, z * a_v4.x - x * a_v4.z, x * a_v4.y - y * a_v4.x, w * a_v4.w);
		}


		float Dot(const Vector4& vecA, const Vector4& a_v3B)
		{
			return(vecA.x * a_v3B.x + vecA.y * a_v3B.y + vecA.z * a_v3B.z, vecA.w * a_v3B.w);
		}

		//returns a bool, if the magnitude is greater than 1 return false
		bool Vector4::IsUnit() const
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

		//used to normalise the vector
		float* Vector4::Normalise()
		{
			Vector4 vector;
			float length = this->Length();

			if (length != 0)
			{
				vector.x = x / length;
				vector.y = y / length;
				vector.z = z / length;
				vector.w = w / length;
			}

			return vector;
		}


		const Vector4 Vector4::GetUnit() const
		{
			float m = Magnitude();
			return Vector4(x / m, y / m, z / m, w / m);
		}

		Vector4 Vector4::GetPerpendicular(const Vector4& a_v4) const
		{
			return (CrossProduct(a_v4));
		}

		//rotates vector on the X axis 
		void Vector4::RotateX(float a_fAngle)
		{
			float cos = cosf(a_fAngle);
			float sin = sinf(a_fAngle);

			Matrix4 TempM(1.f, 0.f, 0.f, 0.f,
				0.f, cos, -sin, 0.f,
				0.f, sin, cos, 0.f,
				0.f, 0.f, 0.f, 1.f);

			Vector4 TempV;
			TempV = TempM * *this;
			*this = TempV;
		}

		//rotates vector on the Y axis 
		void Vector4::RotateY(float a_fAngle)
		{
			float cos = cosf(a_fAngle);
			float sin = sinf(a_fAngle);

			Matrix4 TempM(cos, 0.f, sin, 0.f,
				0.f, 1.f, 0.f, 0.f,
				-sin, 0.f, cos, 0.f,
				0.f, 0.f, 0.f, 1.f);

			Vector4 TempV;
			TempV = TempM * *this;
			*this = TempV;
		}

		//rotates vector on the Z axis 
		void Vector4::RotateZ(float a_fAngle)
		{
			float cos = cosf(a_fAngle);
			float sin = sinf(a_fAngle);

			Matrix4 TempM(cos, -sin, 0.f, 0.f,
				sin, cos, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f);

			Vector4 TempV;
			TempV = TempM * *this;
			*this = TempV;
		}

		//sets all the value to 0
		void Vector4::Zero()
		{
			Vector4(0.0f, 0.0f, 0.0f, 0.f);
		}

		//sets all the values to 1
		void Vector4::One()
		{
			Vector4(1.f, 1.f, 1.f, 1.f);
		}
		float Vector4::Sum() const
		{
			return (x + y + z + w);
		}
		float Vector4::Min() const
		{
			if (x < y && x < z && x < w)
			{
				return x;
			}

			else if (y < x && y < z && y < z)
			{
				return y;
			}

			else if (z < x && z < z && z < w)
			{
				return w;
			}

			else
			{
				return w;
			}
		}

		float Vector4::Max() const
		{
			if (x > y&& x > z&& x > w)
			{
				return x;
			}

			else if (y > x&& y > z&& y > z)
			{
				return y;
			}

			else if (z > x&& z > z&& z > w)
			{
				return w;
			}

			else
			{
				return w;
			}
		}
	}
}