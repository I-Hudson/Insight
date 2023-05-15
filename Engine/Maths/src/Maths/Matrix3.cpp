#include "Maths/Matrix3.h"
#include "Maths/Vector3.h"

#include "Maths/MathsUtils.h"

#include <assert.h>

namespace Insight
{
	namespace Maths
	{
		const Matrix3 Matrix3::Zero     = Matrix3();
		const Matrix3 Matrix3::One      = Matrix3(	1.0f, 1.0f, 1.0f, 
													1.0, 1.0, 1.0, 
													1.0, 1.0, 1.0);
		const Matrix3 Matrix3::Identity = Matrix3(	1.0f, 0.0f, 0.0f, 
													0.0f, 1.0f, 0.0f,
													0.0f, 0.0f, 1.0f);

		Matrix3::Matrix3()
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  1.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 1.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: m_00(0), m_01(0), m_02(0)
			, m_10(0), m_11(0), m_12(0)
			, m_20(0), m_21(0), m_22(0)
#endif
		{ }
		Matrix3::Matrix3(	float m00, float m01, float m02,
							float m10, float m11, float m12,
							float m20, float m21, float m22)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  m00, m01, m02, 0.0f
				, m10, m11, m12, 0.0f
				, m20, m21, m22, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: m_00(m00), m_01(m01), m_02(m02)
			, m_10(m10), m_11(m11), m_12(m12)
			, m_20(m20), m_21(m21), m_22(m22)
#endif
		{ }
		Matrix3::Matrix3(const Matrix3& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01), m_02(other.m_02)
			, m_10(other.m_10), m_11(other.m_11), m_12(other.m_12)
			, m_20(other.m_20), m_21(other.m_21), m_22(other.m_22)
#endif
		{ }
		Matrix3::Matrix3(Matrix3&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01), m_02(other.m_02)
			, m_10(other.m_10), m_11(other.m_11), m_12(other.m_12)
			, m_20(other.m_20), m_21(other.m_21), m_22(other.m_22)
#endif
		{
			other = Matrix3::Zero;
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Matrix3::Matrix3(const DirectX::XMMATRIX& other)
			: xmmatrix(other)
		{ }
		Matrix3::Matrix3(DirectX::XMMATRIX&& other)
			: xmmatrix(other)
		{
			DirectX::XMMatrixSet(
			  1.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 1.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 1.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 1.0f);
		}
#endif

		Matrix3::~Matrix3()
		{ }

		Matrix3 Matrix3::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#else
			// Work out the determinant for each row.
			const float d_00 = m_11 * m_22 - m_21 * m_12;
			const float d_01 = m_10 * m_22 - m_20 * m_12;
			const float d_02 = m_10 * m_21 - m_20 * m_11;

			// Get the determinant for the whole matrix.
			const float determinant = 1.0f / (
				+m_00 * (d_00)
				-m_01 * (d_01)
				+m_02 * (d_02));

			// This matrix is also transposed.
			Matrix3 inverse;
			inverse[0][0] = +(d_00) * determinant;
			inverse[0][1] = -(m_01 * m_22 - m_21 * m_02) * determinant;
			inverse[0][2] = +(m_01 * m_12 - m_11 * m_02) * determinant;

			inverse[1][0] = -(d_01) * determinant;
			inverse[1][1] = +(m_00 * m_22 - m_20 * m_02) * determinant;
			inverse[1][2] = -(m_00 * m_12 - m_10 * m_02) * determinant;

			inverse[2][0] = +(d_02) * determinant;
			inverse[2][1] = -(m_00 * m_21 - m_20 * m_01) * determinant;
			inverse[2][2] = +(m_00 * m_11 - m_10 * m_01) * determinant;

			*this = inverse;
#endif
			return *this;
		}
		Matrix3 Matrix3::Transpose()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = DirectX::XMMatrixTranspose(xmmatrix);
#else
			Matrix3 tran(
				m_00, m_10, m_20,
				m_01, m_11, m_21,
				m_02, m_12, m_22);
			*this = tran;
#endif
			return *this;
		}

		Vector3& Matrix3::operator[](int i)
		{
			switch (i)	
			{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
			default:
				break;
			}
			assert(false);
			return v0;
		}
		Vector3& Matrix3::operator[](unsigned int i)
		{
			switch (i)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
			default:
				break;
			}
			assert(false);
			return v0;
		}
		const Vector3& Matrix3::operator[](int i) const
		{
			return const_cast<Matrix3&>(*this)[i];
		}
		const Vector3& Matrix3::operator[](unsigned int i) const
		{
			return const_cast<Matrix3&>(*this)[i];
		}

		bool Matrix3::operator==(const Matrix3& other) const
		{
			return Equals(m_00, other.m_00, 0.001f) && Equals(m_01, other.m_01, 0.001f) && Equals(m_02, other.m_02, 0.001f)
				&& Equals(m_10, other.m_10, 0.001f) && Equals(m_11, other.m_11, 0.001f) && Equals(m_12, other.m_12, 0.001f)
				&& Equals(m_20, other.m_20, 0.001f) && Equals(m_21, other.m_21, 0.001f) && Equals(m_22, other.m_22, 0.001f);
		}
		bool Matrix3::operator!=(const Matrix3& other) const
		{
			return !(*this == other);
		}

		Matrix3 Matrix3::operator=(const Matrix3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
#else
			m_00 = other.m_00; m_01 = other.m_01; m_02 = other.m_02;
			m_10 = other.m_10; m_11 = other.m_11; m_12 = other.m_12;
			m_20 = other.m_20; m_21 = other.m_21; m_22 = other.m_22;
#endif
			return *this;
		}

		Vector3 Matrix3::operator*(const Vector3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVector3Transform(other.xmvector, xmmatrix));
#else
			const Vector3 vec_z = Vector3(other.z);
			const Vector3 vec_y = Vector3(other.y);
			const Vector3 vec_x = Vector3(other.x);

			Vector3 result;
			result = vec_z * Vector3(v2.x, v2.y, v2.z);
			result = vec_y * Vector3(v1.x, v1.y, v1.z) + result;
			result = vec_x * Vector3(v0.x, v0.y, v0.z) + result;
			return result;
#endif
		}
		Matrix3 Matrix3::operator*(const Matrix3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix3(DirectX::XMMatrixMultiply(xmmatrix, other.xmmatrix));
#else
			Matrix3 m;

			float x = v0[0];
			float y = v0[1];
			float z = v0[2];
			m[0][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z);
			m[0][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z);
			m[0][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z);

			x = v1[0];
			y = v1[1];
			z = v1[2];
			m[1][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z);
			m[1][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z);
			m[1][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z);

			x = v2[0];
			y = v2[1];
			z = v2[2];
			m[2][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z);
			m[2][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z);
			m[2][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z);

			return m;
#endif
		}

		Vector3 Matrix3::operator/(const Vector3& other)
		{
			return Inversed() * other;
		}
		Matrix3 Matrix3::operator/(const Matrix3& other)
		{
			return *this * other.Inversed();
		}

		Matrix3 Matrix3::operator-(const Matrix3& other)
		{
			return Matrix3(
				  m_00 - other.m_00, m_01 - other.m_01, m_02 - other.m_02
				, m_10 - other.m_10, m_11 - other.m_11, m_12 - other.m_12
				, m_20 - other.m_20, m_21 - other.m_21, m_22 - other.m_22);
		}

		Matrix3 Matrix3::operator+(const Matrix3& other)
		{
			return Matrix3(
				m_00 + other.m_00, m_01 + other.m_01, m_02 + other.m_02
				, m_10 + other.m_10, m_11 + other.m_11, m_12 + other.m_12
				, m_20 + other.m_20, m_21 + other.m_21, m_22 + other.m_22);
		}

		Matrix3 Matrix3::operator*=(const Vector3& other)
		{
			(*this)[0] *= other.x;
			(*this)[1] *= other.y;
			(*this)[2] *= other.z;
			return *this;
		}
		Matrix3 Matrix3::operator*=(const Matrix3& other)
		{
			*this = Matrix3(*this) * other;
			return *this;
		}

		Matrix3 Matrix3::operator/=(const Vector3& other)
		{
			(*this)[0] /= other.x;
			(*this)[1] /= other.y;
			(*this)[2] /= other.z;
			return *this;
		}
		Matrix3 Matrix3::operator/=(const Matrix3& other)
		{
			*this = Matrix3(*this) / other;
			return *this;
		}

		Matrix3 Matrix3::operator-=(const Vector3& other)
		{
			(*this)[0] -= other.x;
			(*this)[1] -= other.y;
			(*this)[2] -= other.z;
			return *this;
		}
		Matrix3 Matrix3::operator-=(const Matrix3& other)
		{
			*this = Matrix3(*this) - other;
			return *this;
		}

		Matrix3 Matrix3::operator+=(const Vector3& other)
		{
			(*this)[0] += other.x;
			(*this)[1] += other.y;
			(*this)[2] += other.z;
			return *this;
		}
		Matrix3 Matrix3::operator+=(const Matrix3& other)
		{
			*this = Matrix3(*this) + other;
			return *this;
		}

		const float* Matrix3::Data() const
		{
			return (*this)[0].Data();
		}
	}
}

#ifdef IS_TESTING
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Matrix3")
	{
		const Matrix3 Matrix_Test(
			4, 7, 2,
			2, 6, 12,
			2, -6, 3);
		TEST_CASE("Constructors")
		{
			Matrix3 matrix;
		}

		TEST_CASE("Inverse")
		{
			Matrix3 one = Matrix3(
				12,75,0.45f,
				65,15,8,
				125,40,5);
			Matrix3 inv = one.Inversed();
			Matrix3 result = one * inv;
			CHECK(result == Matrix3::Identity);
		}

		TEST_CASE("Transpose")
		{
			Matrix3 one = Matrix_Test;

			one.Transpose();
			CHECK(Equals(one.m_00, 4.0f));
			CHECK(Equals(one.m_01, 2.0f));
			CHECK(Equals(one.m_02, 2.0f));

			CHECK(Equals(one.m_10, 7.0f));
			CHECK(Equals(one.m_11, 6.0f));
			CHECK(Equals(one.m_12, -6.0f));

			CHECK(Equals(one.m_20, 2.0f));
			CHECK(Equals(one.m_21, 12.0f));
			CHECK(Equals(one.m_22, 3.0f));
		}

		TEST_CASE("operator[]")
		{
			Matrix3 one(
				2, 3, 1,
				7, 4, 1,
				9, -2, 1);
			Vector3 first_row = one[0];
			Vector3 second_row = one[1];
			CHECK(first_row == Vector3(2, 3, 1));
			CHECK(second_row == Vector3(7, 4, 1));

			one[0][1] = 45;
			first_row = one[0];
			CHECK(first_row == Vector3(2, 45, 1));

			Matrix3 inv = one.Inversed();
			Matrix3 result = one * inv;
			CHECK(result == Matrix3::Identity);
		}

		TEST_CASE("Multiplcation")
		{
			Matrix3 one(
				2, 3, 1,
				7, 4, 1,
				9, -2, 1);
			Matrix3 two(
				9, -2, -1,
				5, 7, 3,
				8, 1, 0);

			Matrix3 result = one * two;
			CHECK(Equals(result.m_00, 41.0f, 0.0001f));
			CHECK(Equals(result.m_01, 18.0f, 0.0001f));
			CHECK(Equals(result.m_02, 7.0f, 0.0001f));

			CHECK(Equals(result.m_10, 91.0f, 0.0001f));
			CHECK(Equals(result.m_11, 15.0f, 0.0001f));
			CHECK(Equals(result.m_12, 5.0f, 0.0001f));

			CHECK(Equals(result.m_20, 79.0f, 0.0001f));
			CHECK(Equals(result.m_21, -31.0f, 0.0001f));
			CHECK(Equals(result.m_22, -15.0f, 0.0001f));

			Vector3 vec = Vector3(5, 7, 10);
			result = one *= vec;
			CHECK(Equals(result.m_00, 10.0f, 0.001f));
			CHECK(Equals(result.m_01, 15.0f, 0.001f));
			CHECK(Equals(result.m_02, 5.0f, 0.001f));

			CHECK(Equals(result.m_10, 49.0f, 0.001f));
			CHECK(Equals(result.m_11, 28.0f, 0.001f));
			CHECK(Equals(result.m_12, 7.0f, 0.001f));

			CHECK(Equals(result.m_20, 90.0f, 0.001f));
			CHECK(Equals(result.m_21, -20.0f, 0.001f));
			CHECK(Equals(result.m_22, 10.0f, 0.001f));
			CHECK(result == one);

			one = Matrix3(
				2, 3, 1,
				7, 4, 1,
				9, -2, 1);
			Vector3 result_vec = one * vec;
			CHECK(Equals(result_vec.x, 149.000f, 0.001f));
			CHECK(Equals(result_vec.y, 23.000f, 0.001f));
			CHECK(Equals(result_vec.z, 22.000f, 0.001f));
		}

		TEST_CASE("Divition")
		{
			Matrix3 one(
				2, 3, 1,
				7, 4, 1,
				9, -2, 1);
			Matrix3 two(
				9, -2, -1,
				5, 7, 3,
				8, 1, 0);

			Matrix3 result = one / two;
			CHECK(Equals(result.m_00, -0.625f, 0.001f));
			CHECK(Equals(result.m_01,  0.125f, 0.001f));
			CHECK(Equals(result.m_02,  0.875f, 0.001f));

			CHECK(Equals(result.m_10, -1.000f, 0.001f));
			CHECK(Equals(result.m_11,  0.000f, 0.001f));
			CHECK(Equals(result.m_12,  2.000f, 0.001f));

			CHECK(Equals(result.m_20,  5.250f, 0.001f));
			CHECK(Equals(result.m_21,  2.083f, 0.001f));
			CHECK(Equals(result.m_22, -6.083f, 0.001f));

			Vector3 vec = Vector3(5, 7, 10);
			result = one /= vec;
			CHECK(Equals(result.m_00, 0.400f, 0.001f));
			CHECK(Equals(result.m_01, 0.600f, 0.001f));
			CHECK(Equals(result.m_02, 0.200f, 0.001f));

			CHECK(Equals(result.m_10, 1.0f, 0.001f));
			CHECK(Equals(result.m_11, 0.571f, 0.001f));
			CHECK(Equals(result.m_12, 0.142f, 0.001f));

			CHECK(Equals(result.m_20,  0.900f, 0.001f));
			CHECK(Equals(result.m_21, -0.200f, 0.001f));
			CHECK(Equals(result.m_22,  0.100f, 0.001f));
			CHECK(result == one);
		}

		/*TEST_CASE("Subtraction")
		{
			Matrix3 one = Matrix3(5, 10,
								 25, 9);
			Vector3 vec = Vector3(5, 7);
			Matrix3 result = one -= vec;
			CHECK(Equals(result.m_00, 0.0f, 0.001f));
			CHECK(Equals(result.m_01, 5.0f, 0.001f));
			CHECK(Equals(result.m_10, 18.0f, 0.001f));
			CHECK(Equals(result.m_11, 2.0f, 0.001f));
			CHECK(result == one);
		}*/
	}
}
#endif