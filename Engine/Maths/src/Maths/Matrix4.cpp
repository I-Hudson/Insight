#include "Maths/Matrix4.h"
#include "Maths/Vector4.h"

#include "Maths/MathsUtils.h"

namespace Insight
{
	namespace Maths
	{
		const Matrix4 Matrix4::Zero     = Matrix4();
		const Matrix4 Matrix4::One		= Matrix4(
												1.0f, 1.0f, 1.0f, 1.0f,
												1.0f, 1.0f, 1.0f, 1.0f,
												1.0f, 1.0f, 1.0f, 1.0f,
												1.0f, 1.0f, 1.0f, 1.0f);

		const Matrix4 Matrix4::Identity = Matrix4(	1.0f, 0.0f, 0.0f, 0.0f, 
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f,
													0.0f, 0.0f, 0.0f, 1.0f);

		Matrix4::Matrix4()
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  1.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 1.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: m_00(0.0f), m_01(0.0f), m_02(0.0f), m_03(0.0f)
			, m_10(0.0f), m_11(0.0f), m_12(0.0f), m_13(0.0f)
			, m_20(0.0f), m_21(0.0f), m_22(0.0f), m_23(0.0f)
			, m_30(0.0f), m_31(0.0f), m_32(0.0f), m_33(0.0f)
#endif
		{
			data[0] = Vector4::Zero;
			data[1] = Vector4::Zero;
			data[2] = Vector4::Zero;
			data[3] = Vector4::Zero;
		}
		Matrix4::Matrix4(	float m00, float m01, float m02, float m03,
							float m10, float m11, float m12, float m13,
							float m20, float m21, float m22, float m23,
							float m30, float m31, float m32, float m33)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  m00, m01, m02, m03
				, m10, m11, m12, m13
				, m20, m21, m22, m23
				, m30, m31, m32, m33))
#else
			: m_00(m00), m_01(m01), m_02(m02), m_03(m03)
			, m_10(m10), m_11(m11), m_12(m12), m_13(m13)
			, m_20(m20), m_21(m21), m_22(m22), m_23(m23)
			, m_30(m30), m_31(m31), m_32(m32), m_33(m33)
#endif
		{
			data[0] = Vector4(m00, m01, m02, m03);
			data[1] = Vector4(m10, m11, m12, m13);
			data[2] = Vector4(m20, m21, m22, m23);
			data[3] = Vector4(m30, m31, m32, m33);
		}
		Matrix4::Matrix4(const Matrix4& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01), m_02(other.m_02), m_03(other.m_03)
			, m_10(other.m_10), m_11(other.m_11), m_12(other.m_12), m_13(other.m_13)
			, m_20(other.m_20), m_21(other.m_21), m_22(other.m_22), m_23(other.m_23)
			, m_30(other.m_30), m_31(other.m_31), m_32(other.m_32), m_33(other.m_33)
#endif
		{ }
		Matrix4::Matrix4(Matrix4&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01), m_02(other.m_02), m_03(other.m_03)
			, m_10(other.m_10), m_11(other.m_11), m_12(other.m_12), m_13(other.m_13)
			, m_20(other.m_20), m_21(other.m_21), m_22(other.m_22), m_23(other.m_23)
			, m_30(other.m_30), m_31(other.m_31), m_32(other.m_32), m_33(other.m_33)
#endif
		{
			other = Matrix4::Zero;
		}

#ifdef IS_MATHS_DIRECTX_MATHS
		Matrix4::Matrix4(const DirectX::XMMATRIX& other)
			: xmmatrix(other)
		{ }
		Matrix4::Matrix4(DirectX::XMMATRIX&& other)
			: xmmatrix(other)
		{
			DirectX::XMMatrixSet(
			  1.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 1.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 1.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 1.0f);
		}
#endif

		Matrix4::~Matrix4()
		{ }

		Matrix4 Matrix4::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#else
			Matrix4 tran = Transposed();

			Vector4 v0[4], v1[4];
			v0[0] = Vector4(tran.m_20, tran.m_20, tran.m_21, tran.m_21);
			v1[0] = Vector4(tran.m_32, tran.m_33, tran.m_32, tran.m_33);
			v0[1] = Vector4(tran.m_00, tran.m_00, tran.m_01, tran.m_01);
			v1[1] = Vector4(tran.m_12, tran.m_13, tran.m_12, tran.m_13);
			v0[2] = Vector4(tran.m_20, tran.m_22, tran.m_00, tran.m_02);
			v1[2] = Vector4(tran.m_31, tran.m_33, tran.m_11, tran.m_13);

			Vector4 d0 = v0[0] * v1[0];
			Vector4 d1 = v0[1] * v1[1];
			Vector4 d2 = v0[2] * v1[2];

			v0[0] = Vector4(tran.m_22, tran.m_23, tran.m_22, tran.m_23);
			v1[0] = Vector4(tran.m_30, tran.m_30, tran.m_31, tran.m_31);
			v0[1] = Vector4(tran.m_02, tran.m_03, tran.m_02, tran.m_03);
			v1[1] = Vector4(tran.m_10, tran.m_10, tran.m_11, tran.m_11);
			v0[2] = Vector4(tran.m_21, tran.m_23, tran.m_01, tran.m_03);
			v1[2] = Vector4(tran.m_30, tran.m_32, tran.m_10, tran.m_12);

			d0 = d0 - (v0[0] * v1[0]);
			d1 = d1 - (v0[1] * v1[1]);
			d2 = d2 - (v0[2] * v1[2]);

			v0[0] = Vector4(tran.m_11, tran.m_12, tran.m_10, tran.m_11);
			v1[0] = Vector4(d2.y, d0.y, d0.w, d0.x);
			v0[1] = Vector4(tran.m_02, tran.m_00, tran.m_01, tran.m_00);
			v1[1] = Vector4(d0.w, d2.y, d0.y, d0.z);
			v0[2] = Vector4(tran.m_31, tran.m_32, tran.m_30, tran.m_31);
			v1[2] = Vector4(d2.w, d1.y, d1.w, d1.x);
			v0[3] = Vector4(tran.m_22, tran.m_20, tran.m_21, tran.m_20);
			v1[3] = Vector4(d1.w, d2.w, d1.y, d1.z);

			Vector4 c0 = v0[0] * v1[0];
			Vector4 c2 = v0[1] * v1[1];
			Vector4 c4 = v0[2] * v1[2];
			Vector4 c6 = v0[3] * v1[3];

			v0[0] = Vector4(tran.m_12, tran.m_13, tran.m_11, tran.m_12);
			v1[0] = Vector4(d0.w, d0.x, d0.y, d2.x);
			v0[1] = Vector4(tran.m_03, tran.m_02, tran.m_03, tran.m_01);
			v1[1] = Vector4(d0.z, d0.y, d2.x, d0.x);
			v0[2] = Vector4(tran.m_32, tran.m_33, tran.m_31, tran.m_32);
			v1[2] = Vector4(d1.w, d1.x, d1.y, d2.z);
			v0[3] = Vector4(tran.m_23, tran.m_22, tran.m_23, tran.m_21);
			v1[3] = Vector4(d1.z, d1.y, d2.z, d1.x);

			c0 = c0 - (v0[0] * v1[0]);
			c2 = c2 - (v0[1] * v1[1]);
			c4 = c4 - (v0[2] * v1[2]);
			c6 = c6 - (v0[3] * v1[3]);

			v0[0] = Vector4(tran.m_13, tran.m_10, tran.m_13, tran.m_10);
			v1[0] = Vector4(d0.z, d2.y, d2.x, d0.z);
			v0[1] = Vector4(tran.m_01, tran.m_03, tran.m_00, tran.m_02);
			v1[1] = Vector4(d2.y, d0.x, d0.w, d2.x);
			v0[2] = Vector4(tran.m_33, tran.m_30, tran.m_33, tran.m_30);
			v1[2] = Vector4(d1.z, d2.w, d2.z, d1.z);
			v0[3] = Vector4(tran.m_21, tran.m_23, tran.m_20, tran.m_22);
			v1[3] = Vector4(d2.w, d1.x, d1.w, d2.z);

			Vector4 c1 = c0 - (v0[0] * v1[0]);
			c0 = (v0[0] * v1[0]) + c0;

			Vector4 c3 = (v0[1] * v1[1]) + c2;
			c2 = c2 - (v0[1] * v1[1]);

			Vector4 c5 = c4 - (v0[2] * v1[2]);
			c4 = (v0[2] * v1[2]) + c4;

			Vector4 c7 = (v0[3] * v1[3]) + c6;
			c6 = c6 - (v0[3] * v1[3]);

			Matrix4 r;
			r[0] = Vector4(c0.x, c1.y, c0.z, c1.w);
			r[1] = Vector4(c2.x, c3.y, c2.z, c3.w);
			r[2] = Vector4(c4.x, c5.y, c4.z, c5.w);
			r[3] = Vector4(c6.x, c7.y, c6.z, c7.w);

			Vector4 determinant = Vector4(r[0]).Dot(tran[0]);

			Vector4 reciporcal(
				1.0f / determinant.x,
				1.0f / determinant.y,
				1.0f / determinant.z,
				1.0f / determinant.w);

			Matrix4 m;
			m[0] = r[0] * reciporcal;
			m[1] = r[1] * reciporcal;
			m[2] = r[2] * reciporcal;
			m[3] = r[3] * reciporcal;

			*this = m;
#endif
			return *this;
		}
		Matrix4 Matrix4::Transpose()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = DirectX::XMMatrixTranspose(xmmatrix);
#else
			Matrix4 tran(
				m_00, m_10, m_20, m_30, 
				m_01, m_11, m_21, m_31,
				m_02, m_12, m_22, m_32,
				m_03, m_13, m_23, m_33);
			*this = tran;
#endif
			return *this;
		}

		Vector4& Matrix4::operator[](int i)
		{
			return data[i];
		}
		Vector4& Matrix4::operator[](unsigned int i)
		{
			return data[i];
		}
		const Vector4& Matrix4::operator[](int i) const
		{
			return data[i];
		}
		const Vector4& Matrix4::operator[](unsigned int i) const
		{
			return data[i];
		}

		bool Matrix4::operator==(const Matrix4& other) const
		{
			return Equals(m_00, other.m_00, 0.001f) && Equals(m_01, other.m_01, 0.001f) && Equals(m_02, other.m_02, 0.001f) && Equals(m_03, other.m_03, 0.001f)
				&& Equals(m_10, other.m_10, 0.001f) && Equals(m_11, other.m_11, 0.001f) && Equals(m_12, other.m_12, 0.001f) && Equals(m_13, other.m_13, 0.001f)
				&& Equals(m_20, other.m_20, 0.001f) && Equals(m_21, other.m_21, 0.001f) && Equals(m_22, other.m_22, 0.001f) && Equals(m_23, other.m_23, 0.001f)
				&& Equals(m_30, other.m_30, 0.001f) && Equals(m_31, other.m_31, 0.001f) && Equals(m_32, other.m_32, 0.001f) && Equals(m_33, other.m_33, 0.001f);
		}
		bool Matrix4::operator!=(const Matrix4& other) const
		{
			return !(*this == other);
		}

		Matrix4 Matrix4::operator=(const Matrix4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
#else
			m_00 = other.m_00; m_01 = other.m_01; m_02 = other.m_02; m_03 = other.m_03;
			m_10 = other.m_10; m_11 = other.m_11; m_12 = other.m_12; m_13 = other.m_13;
			m_20 = other.m_20; m_21 = other.m_21; m_22 = other.m_22; m_23 = other.m_23;
			m_30 = other.m_30; m_31 = other.m_31; m_32 = other.m_32; m_33 = other.m_33;
#endif
			return *this;
		}

		Vector4 Matrix4::operator*(const Vector4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector4(DirectX::XMVector4Transform(other.xmvector, xmmatrix));
#else
			const float x = (m_00 * other.x) + (m_10 * other.y) + (m_20 * other.z) + (m_30 * other.w);
			const float y = (m_01 * other.x) + (m_11 * other.y) + (m_21 * other.z) + (m_31 * other.w);
			const float z = (m_02 * other.x) + (m_12 * other.y) + (m_22 * other.z) + (m_32 * other.w);
			const float w = (m_03 * other.x) + (m_13 * other.y) + (m_23 * other.z) + (m_33 * other.w);
			return Vector4(x, y, z, w);
#endif
		}
		Matrix4 Matrix4::operator*(const Matrix4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix4(DirectX::XMMatrixMultiply(xmmatrix, other.xmmatrix));
#else
			Matrix4 m;

			float x = data[0][0];
			float y = data[0][1];
			float z = data[0][2];
			float w = data[0][3];
			m[0][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z) + (other.m_30 * w);
			m[0][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z) + (other.m_31 * w);
			m[0][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z) + (other.m_32 * w);
			m[0][3] = (other.m_03 * x) + (other.m_13 * y) + (other.m_23 * z) + (other.m_33 * w);

			x = data[1][0];
			y = data[1][1];
			z = data[1][2];
			w = data[1][3];
			m[1][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z) + (other.m_30 * w);
			m[1][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z) + (other.m_31 * w);
			m[1][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z) + (other.m_32 * w);
			m[1][3] = (other.m_03 * x) + (other.m_13 * y) + (other.m_23 * z) + (other.m_33 * w);

			x = data[2][0];
			y = data[2][1];
			z = data[2][2];
			w = data[2][3];
			m[2][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z) + (other.m_30 * w);
			m[2][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z) + (other.m_31 * w);
			m[2][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z) + (other.m_32 * w);
			m[2][3] = (other.m_03 * x) + (other.m_13 * y) + (other.m_23 * z) + (other.m_33 * w);

			x = data[3][0];
			y = data[3][1];
			z = data[3][2];
			w = data[3][3];
			m[3][0] = (other.m_00 * x) + (other.m_10 * y) + (other.m_20 * z) + (other.m_30 * w);
			m[3][1] = (other.m_01 * x) + (other.m_11 * y) + (other.m_21 * z) + (other.m_31 * w);
			m[3][2] = (other.m_02 * x) + (other.m_12 * y) + (other.m_22 * z) + (other.m_32 * w);
			m[3][3] = (other.m_03 * x) + (other.m_13 * y) + (other.m_23 * z) + (other.m_33 * w);

			return m;
#endif
		}

		Vector4 Matrix4::operator/(const Vector4& other)
		{
			return Inversed() * other;
		}
		Matrix4 Matrix4::operator/(const Matrix4& other)
		{
			return *this * other.Inversed();
		}

		Matrix4 Matrix4::operator-(const Matrix4& other)
		{
			return Matrix4(
				  m_00 - other.m_00, m_01 - other.m_01, m_02 - other.m_02, m_03 - other.m_03
				, m_10 - other.m_10, m_11 - other.m_11, m_12 - other.m_12, m_13 - other.m_13
				, m_20 - other.m_20, m_21 - other.m_21, m_22 - other.m_22, m_23 - other.m_23
				, m_30 - other.m_30, m_31 - other.m_31, m_32 - other.m_32, m_33 - other.m_33);
		}

		Matrix4 Matrix4::operator+(const Matrix4& other)
		{
			return Matrix4(
				  m_00 + other.m_00, m_01 + other.m_01, m_02 + other.m_02, m_03 + other.m_03
				, m_10 + other.m_10, m_11 + other.m_11, m_12 + other.m_12, m_13 + other.m_13
				, m_20 + other.m_20, m_21 + other.m_21, m_22 + other.m_22, m_23 + other.m_23
				, m_30 + other.m_30, m_31 + other.m_31, m_32 + other.m_32, m_33 + other.m_33);
		}

		Matrix4 Matrix4::operator*=(const Vector4& other)
		{
			data[0] *= other.x;
			data[1] *= other.y;
			data[2] *= other.z;
			data[3] *= other.w;
			return *this;
		}
		Matrix4 Matrix4::operator*=(const Matrix4& other)
		{
			*this = Matrix4(*this) * other;
			return *this;
		}

		Matrix4 Matrix4::operator/=(const Vector4& other)
		{
			data[0] /= other.x;
			data[1] /= other.y;
			data[2] /= other.z;
			data[3] /= other.w;
			return *this;
		}
		Matrix4 Matrix4::operator/=(const Matrix4& other)
		{
			*this = Matrix4(*this) / other;
			return *this;
		}

		Matrix4 Matrix4::operator-=(const Vector4& other)
		{
			data[0] -= other.x;
			data[1] -= other.y;
			data[2] -= other.z;
			data[3] -= other.w;
			return *this;
		}
		Matrix4 Matrix4::operator-=(const Matrix4& other)
		{
			*this = Matrix4(*this) - other;
			return *this;
		}

		Matrix4 Matrix4::operator+=(const Vector4& other)
		{
			data[0] += other.x;
			data[1] += other.y;
			data[2] += other.z;
			data[3] += other.w;
			return *this;
		}
		Matrix4 Matrix4::operator+=(const Matrix4& other)
		{
			*this = Matrix4(*this) + other;
			return *this;
		}

		const float* Matrix4::Data() const
		{
			return data[0].Data();
		}
	}
}

#ifdef TEST_ENABLED
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Matrix4")
	{
		TEST_CASE("Constructors")
		{
		}

		TEST_CASE("Inverse")
		{
			Matrix4 matrix(
				4, 5, 3, 7,
				5, 2, 4, 9,
				10, 2, 3, 1,
				1, 5, 7, 1);
			Matrix4 inv = matrix.Inversed();
			Matrix4 result = matrix * inv;
			CHECK(result == Matrix4::Identity);
		}

		TEST_CASE("Transpose")
		{
			Matrix4 matrix(
				4, 5, 3, 7,
				5, 2, 4, 9,
				10, 2, 3, 1,
				1, 5, 7, 1);
			matrix.Transpose();
			CHECK(Equals(matrix.m_00, 4.0f));
			CHECK(Equals(matrix.m_01, 5.0f));
			CHECK(Equals(matrix.m_02, 10.0f));
			CHECK(Equals(matrix.m_03, 1.0f));

			CHECK(Equals(matrix.m_10, 5.0f));
			CHECK(Equals(matrix.m_11, 2.0f));
			CHECK(Equals(matrix.m_12, 2.0f));
			CHECK(Equals(matrix.m_13, 5.0f));

			CHECK(Equals(matrix.m_20, 3.0f));
			CHECK(Equals(matrix.m_21, 4.0f));
			CHECK(Equals(matrix.m_22, 3.0f));
			CHECK(Equals(matrix.m_23, 7.0f));

			CHECK(Equals(matrix.m_30, 7.0f));
			CHECK(Equals(matrix.m_31, 9.0f));
			CHECK(Equals(matrix.m_32, 1.0f));
			CHECK(Equals(matrix.m_33, 1.0f));
		}

		TEST_CASE("operator[]")
		{
			Matrix4 matrix(
				4, 5, 3, 7,
				5, 2, 4, 9,
				10, 2, 3, 1,
				1, 5, 7, 1);
			Vector4 first_row = matrix[0];
			Vector4 second_row = matrix[1];
			CHECK(first_row == Vector4(4, 5, 3, 7));
			CHECK(second_row == Vector4(5, 2, 4, 9));

			matrix[0][1] = 45;
			first_row = matrix[0];
			CHECK(first_row == Vector4(4, 45, 3, 7));

			Matrix4 inv = matrix.Inversed();
			Matrix4 result = matrix * inv;
			CHECK(result == Matrix4::Identity);
		}

		TEST_CASE("Multiplcation")
		{
			Matrix4 one(
				4, 5, 3, 7,
				5, 2, 4, 9,
				10, 2, 3, 1,
				1, 5, 7, 1);
			Matrix4 two(
				1, 2, 8, 3,
				2, 4, 4, 1,
				20, 1, 10, 1,
				1, 5, 7, 1);

			Matrix4 result = one * two;
			CHECK(Equals(result.m_00, 81.000f, 0.0001f));
			CHECK(Equals(result.m_01, 66.000f, 0.0001f));
			CHECK(Equals(result.m_02, 131.000f, 0.0001f));
			CHECK(Equals(result.m_03, 27.000f, 0.0001f));

			CHECK(Equals(result.m_10,  98.000f, 0.0001f));
			CHECK(Equals(result.m_11,  67.000f, 0.0001f));
			CHECK(Equals(result.m_12, 151.000f, 0.0001f));
			CHECK(Equals(result.m_13,  30.000f, 0.0001f));

			CHECK(Equals(result.m_20,  75.000f, 0.0001f));
			CHECK(Equals(result.m_21,  36.000f, 0.0001f));
			CHECK(Equals(result.m_22, 125.000f, 0.0001f));
			CHECK(Equals(result.m_23,  36.000f, 0.0001f));

			CHECK(Equals(result.m_30, 152.000f, 0.0001f));
			CHECK(Equals(result.m_31,  34.000f, 0.0001f));
			CHECK(Equals(result.m_32, 105.00f, 0.0001f));
			CHECK(Equals(result.m_33,  16.000f, 0.0001f));

			Vector4 vec = Vector4(5, 7, 10, 1);
			result = one *= vec;
			CHECK(Equals(result.m_00, 20.000f, 0.001f));
			CHECK(Equals(result.m_01, 25.000f, 0.001f));
			CHECK(Equals(result.m_02, 15.000f, 0.001f));
			CHECK(Equals(result.m_03, 35.00f, 0.001f));

			CHECK(Equals(result.m_10, 35.000f, 0.001f));
			CHECK(Equals(result.m_11, 14.000f, 0.001f));
			CHECK(Equals(result.m_12, 28.000f, 0.001f));
			CHECK(Equals(result.m_13, 63.000f, 0.001f));

			CHECK(Equals(result.m_20, 100.000f, 0.001f));
			CHECK(Equals(result.m_21, 20.000f, 0.001f));
			CHECK(Equals(result.m_22, 30.000f, 0.001f));
			CHECK(Equals(result.m_23, 10.000f, 0.001f));

			CHECK(Equals(result.m_30, 1.000f, 0.001f));
			CHECK(Equals(result.m_31, 5.000f, 0.001f));
			CHECK(Equals(result.m_32, 7.000f, 0.001f));
			CHECK(Equals(result.m_33, 1.000f, 0.001f));

			CHECK(result == one);

			one = Matrix4(
				4, 5, 3, 7,
				5, 2, 4, 9,
				10, 2, 3, 1,
				1, 5, 7, 1);
			Vector4 result_vec = one * vec;
			CHECK(Equals(result_vec.x, 156.000f, 0.001f));
			CHECK(Equals(result_vec.y,  64.000f, 0.001f));
			CHECK(Equals(result_vec.z,  80.000f, 0.001f));
			CHECK(Equals(result_vec.w, 109.000f, 0.001f));
		}

		/*TEST_CASE("Divition")
		{
			Matrix4 one(
				2, 3, 1,
				7, 4, 1,
				9, -2, 1);
			Matrix4 two(
				9, -2, -1,
				5, 7, 3,
				8, 1, 0);

			Matrix4 result = one / two;
			CHECK(Equals(result.m_00, -0.625f, 0.001f));
			CHECK(Equals(result.m_01,  0.125f, 0.001f));
			CHECK(Equals(result.m_02,  0.875f, 0.001f));

			CHECK(Equals(result.m_10, -1.000f, 0.001f));
			CHECK(Equals(result.m_11,  0.000f, 0.001f));
			CHECK(Equals(result.m_12,  2.000f, 0.001f));

			CHECK(Equals(result.m_20,  5.250f, 0.001f));
			CHECK(Equals(result.m_21,  2.083f, 0.001f));
			CHECK(Equals(result.m_22, -6.083f, 0.001f));

			Vector4 vec = Vector4(5, 7, 10);
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
		}*/

		/*TEST_CASE("Subtraction")
		{
			Matrix4 one = Matrix4(5, 10,
								 25, 9);
			Vector4 vec = Vector4(5, 7);
			Matrix4 result = one -= vec;
			CHECK(Equals(result.m_00, 0.0f, 0.001f));
			CHECK(Equals(result.m_01, 5.0f, 0.001f));
			CHECK(Equals(result.m_10, 18.0f, 0.001f));
			CHECK(Equals(result.m_11, 2.0f, 0.001f));
			CHECK(result == one);
		}*/
	}
}
#endif