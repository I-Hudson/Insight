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
				  0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: m_00(0), m_01(0), m_02(0)
			, m_10(0), m_11(0), m_12(0)
			, m_20(0), m_21(0), m_22(0)
#endif
		{ }

		Matrix3::Matrix3(Vector3 v0, Vector3 v1, Vector3 v2)
			: v0(v0)
			, v1(v1)
			, v2(v2)
		{ }

		Matrix3::Matrix3(const Quaternion & q)
		{
			*this = Identity;

			const float qxx(q.x * q.x);
			const float qyy(q.y * q.y);
			const float qzz(q.z * q.z);
			const float qxz(q.x * q.z);
			const float qxy(q.x * q.y);
			const float qyz(q.y * q.z);
			const float qwx(q.w * q.x);
			const float qwy(q.w * q.y);
			const float qwz(q.w * q.z);

			m_00 = 1.0f - 2.0f * (qyy + qzz);
			m_01 = 2.0f * (qxy + qwz);
			m_02 = 2.0f * (qxz - qwy);

			m_10 = 2.0f * (qxy - qwz);
			m_11 = 1.0f - 2.0f * (qxx + qzz);
			m_12 = 2.0f * (qyz + qwx);

			m_20 = 2.0f * (qxz + qwy);
			m_21 = 2.0f * (qyz - qwx);
			m_22 = 1.0f - 2.0f * (qxx + qyy);
		}

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
		{ }
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)

		Matrix3::Matrix3(const glm::mat3& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			/*
			* Refer to Matrix2.
			*/
			: xmmatrix(DirectX::XMMatrixSet(
				  other[0].x, other[0].y, other[0].z, 0.0f
				, other[1].x, other[1].y, other[1].z, 0.0f
				, other[2].x, other[2].y, other[2].z, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: mat3(other)
#endif
		{ }
		Matrix3::Matrix3(glm::mat3&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  other[0].x, other[0].y, other[0].z, 0.0f
				, other[1].x, other[1].y, other[1].z, 0.0f
				, other[2].x, other[2].y, other[2].z, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: mat3(other)
#endif
		{ }
#endif

		Matrix3::~Matrix3()
		{ }

		Matrix3& Matrix3::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#elif defined(IS_MATHS_GLM)
			mat3 = glm::inverse(mat3);
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
		Matrix3& Matrix3::Transpose()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = DirectX::XMMatrixTranspose(xmmatrix);
#elif defined(IS_MATHS_GLM)
			mat3 = glm::transpose(mat3);
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
		const Vector3& Matrix3::operator[](int i) const
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

		bool Matrix3::Equal(const Matrix3& other, const float errorRange) const
		{
			return v0.Equal(other.v0, errorRange) && v1.Equal(other.v1, errorRange) && v2.Equal(other.v2, errorRange);
		}

		bool Matrix3::NotEqual(const Matrix3& other, const float errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		Matrix3& Matrix3::operator=(const Matrix3& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
#elif defined(IS_MATHS_GLM)
			mat3 = other.mat3;
#else
			m_00 = other.m_00; m_01 = other.m_01; m_02 = other.m_02;
			m_10 = other.m_10; m_11 = other.m_11; m_12 = other.m_12;
			m_20 = other.m_20; m_21 = other.m_21; m_22 = other.m_22;
#endif
			return *this;
		}

		Vector3 Matrix3::operator*(const Vector3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector3(DirectX::XMVector3Transform(other.xmvector, xmmatrix));
#elif defined(IS_MATHS_GLM)
			return mat3 * other.vec3;
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
		Matrix3 Matrix3::operator*(const Matrix3& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix3(DirectX::XMMatrixMultiply(other.xmmatrix, xmmatrix));
#elif defined(IS_MATHS_GLM)
			return mat3 * other.mat3;
#else
			Matrix3 m;

			float x = other[0][0];
			float y = other[0][1];
			float z = other[0][2];
			m[0][0] = (x * m_00) + (y * m_10) + (z * m_20);
			m[0][1] = (x * m_01) + (y * m_11) + (z * m_21);
			m[0][2] = (x * m_02) + (y * m_12) + (z * m_22 );

			x = other[1][0];
			y = other[1][1];
			z = other[1][2];
			m[1][0] = (x * m_00) + (y * m_10) + (z * m_20);
			m[1][1] = (x * m_01) + (y * m_11) + (z * m_21);
			m[1][2] = (x * m_02) + (y * m_12) + (z * m_22);

			x = other[2][0];
			y = other[2][1];
			z = other[2][2];
			m[2][0] = (x * m_00) + (y * m_10) + (z * m_20);
			m[2][1] = (x * m_01) + (y * m_11) + (z * m_21);
			m[2][2] = (x * m_02) + (y * m_12) + (z * m_22);

			return m;
#endif
		}

		Vector3 Matrix3::operator/(const Vector3& other) const
		{
			return Inversed() * other;
		}
		Matrix3 Matrix3::operator/(const Matrix3& other) const
		{
			return *this * other.Inversed();
		}

		Matrix3 Matrix3::operator-(const Matrix3& other) const
		{
			return Matrix3(
				v0 - other.v0,
				v1 - other.v1,
				v2 - other.v2);
		}

		Matrix3 Matrix3::operator+(const Matrix3& other) const
		{
			return Matrix3(
				v0 + other.v0,
				v1 + other.v1,
				v2 + other.v2);
		}

		Matrix3& Matrix3::operator*=(const Vector3& other)
		{
			(*this)[0] *= other;
			(*this)[1] *= other;
			(*this)[2] *= other;
			return *this;
		}
		Matrix3& Matrix3::operator*=(const Matrix3& other)
		{
			*this = Matrix3(*this) * other;
			return *this;
		}

		Matrix3& Matrix3::operator/=(const Vector3& other)
		{
			(*this)[0] /= other;
			(*this)[1] /= other;
			(*this)[2] /= other;
			return *this;
		}
		Matrix3& Matrix3::operator/=(const Matrix3& other)
		{
			*this = Matrix3(*this) / other;
			return *this;
		}

		Matrix3& Matrix3::operator-=(const Vector3& other)
		{
			(*this)[0] -= other;
			(*this)[1] -= other;
			(*this)[2] -= other;
			return *this;
		}
		Matrix3& Matrix3::operator-=(const Matrix3& other)
		{
			*this = Matrix3(*this) - other;
			return *this;
		}

		Matrix3& Matrix3::operator+=(const Vector3& other)
		{
			(*this)[0] += other;
			(*this)[1] += other;
			(*this)[2] += other;
			return *this;
		}
		Matrix3& Matrix3::operator+=(const Matrix3& other)
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
		const float MatrixText_X1 = 4;
		const float MatrixText_Y1 = 7;
		const float MatrixText_Z1 = 3;

		const float MatrixText_X2 = 2;
		const float MatrixText_Y2 = 6;
		const float MatrixText_Z2 = 9;

		const float MatrixText_X3 = 127;
		const float MatrixText_Y3 = 26;
		const float MatrixText_Z3 = 87;

		const float MatrixText_X4 = 785;
		const float MatrixText_Y4 = 1249;
		const float MatrixText_Z4 = 8613;

		const float MatrixText_X5 = 578;
		const float MatrixText_Y5 = 231;
		const float MatrixText_Z5 = 85;

		const float MatrixText_X6 = 983;
		const float MatrixText_Y6 = 85419;
		const float MatrixText_Z6 = 826;

		const glm::mat3 glmMatrixTest(
			MatrixText_X1, MatrixText_Y1, MatrixText_Z1,
			MatrixText_X2, MatrixText_Y2, MatrixText_Z2,
			MatrixText_X3, MatrixText_Y3, MatrixText_Z3);

		const Matrix3 Matrix_Test(
			MatrixText_X1, MatrixText_Y1, MatrixText_Z1,
			MatrixText_X2, MatrixText_Y2, MatrixText_Z2,
			MatrixText_X3, MatrixText_Y3, MatrixText_Z3);


		const glm::mat3 glmMatrixTest1(
			MatrixText_X4, MatrixText_Y4, MatrixText_Z4,
			MatrixText_X5, MatrixText_Y5, MatrixText_Z5,
			MatrixText_X6, MatrixText_Y6, MatrixText_Z6);

		const Matrix3 Matrix_Test1(
			MatrixText_X4, MatrixText_Y4, MatrixText_Z4,
			MatrixText_X5, MatrixText_Y5, MatrixText_Z5,
			MatrixText_X6, MatrixText_Y6, MatrixText_Z6);

		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Matrix3) == 64);
#else
			CHECK(sizeof(Matrix3) == 36);
#endif
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float z1 = 0.0f;

			float x2 = 0.0f;
			float y2 = 0.0f;
			float z2 = 0.0f;

			float x3 = 0.0f;
			float y3 = 0.0f;
			float z3 = 0.0f;

			Matrix3 matrix;
			{
				CHECK(matrix[0].x == x1);
				CHECK(matrix[0].y == y1);
				CHECK(matrix[0].z == z1);

				CHECK(matrix[1].x == x2);
				CHECK(matrix[1].y == y2);
				CHECK(matrix[1].z == z2);

				CHECK(matrix[2].x == x3);
				CHECK(matrix[2].y == y3);
				CHECK(matrix[2].z == z3);
			}

			{
				CHECK(matrix.m_00 == x1);
				CHECK(matrix.m_01 == y1);
				CHECK(matrix.m_02 == z1);

				CHECK(matrix.m_10 == x2);
				CHECK(matrix.m_11 == y2);
				CHECK(matrix.m_12 == z2);

				CHECK(matrix.m_20 == x3);
				CHECK(matrix.m_21 == y3);
				CHECK(matrix.m_22 == z3);
			}

			x1 = 1.0f;
			y1 = 2.0f;
			z1 = 3.0f;

			x2 = 4.0f;
			y2 = 5.0f;
			z2 = 6.0f;

			matrix = Matrix3(x1, y1, z1, x2, y2, z2, x3, y3, z3);
			{
				CHECK(matrix[0].x == x1);
				CHECK(matrix[0].y == y1);
				CHECK(matrix[0].z == z1);

				CHECK(matrix[1].x == x2);
				CHECK(matrix[1].y == y2);
				CHECK(matrix[1].z == z2);

				CHECK(matrix[2].x == x3);
				CHECK(matrix[2].y == y3);
				CHECK(matrix[2].z == z3);
			}

			{
				CHECK(matrix.m_00 == x1);
				CHECK(matrix.m_01 == y1);
				CHECK(matrix.m_02 == z1);

				CHECK(matrix.m_10 == x2);
				CHECK(matrix.m_11 == y2);
				CHECK(matrix.m_12 == z2);

				CHECK(matrix.m_20 == x3);
				CHECK(matrix.m_21 == y3);
				CHECK(matrix.m_22 == z3);
			}

			Matrix3 matrix2(matrix);
			{
				CHECK(matrix2[0].x == x1);
				CHECK(matrix2[0].y == y1);
				CHECK(matrix2[0].z == z1);

				CHECK(matrix2[1].x == x2);
				CHECK(matrix2[1].y == y2);
				CHECK(matrix2[1].z == z2);

				CHECK(matrix2[2].x == x3);
				CHECK(matrix2[2].y == y3);
				CHECK(matrix2[2].z == z3);
			}

			{
				CHECK(matrix2.m_00 == x1);
				CHECK(matrix2.m_01 == y1);
				CHECK(matrix2.m_02 == z1);

				CHECK(matrix2.m_10 == x2);
				CHECK(matrix2.m_11 == y2);
				CHECK(matrix2.m_12 == z2);

				CHECK(matrix2.m_20 == x3);
				CHECK(matrix2.m_21 == y3);
				CHECK(matrix2.m_22 == z3);
			}
		}

		TEST_CASE("Inverse")
		{
			Matrix3 one = Matrix_Test;
			Matrix3 inv = one.Inversed();
			glm::mat3 glmInv = glm::inverse(glmMatrixTest);
			CHECK(inv.Equal(glmInv, 0.0001f));

			Matrix3 result = inv * one;
			CHECK(result.Equal(Matrix3::Identity, 0.0001f));
		}

		TEST_CASE("Transpose")
		{
			CHECK(Matrix_Test.Transposed() == glm::transpose(glmMatrixTest));
			Matrix3 mat = Matrix_Test;
			mat.Transpose();
			CHECK(mat == glm::transpose(glmMatrixTest));
		}

		TEST_CASE("operator[]")
		{
			Matrix3 one = Matrix_Test;
			Vector3 first_row = one[0];
			Vector3 second_row = one[1];
			Vector3 thrid_row = one[2];
			CHECK(first_row == Matrix_Test[0]);
			CHECK(second_row == Matrix_Test[1]);
			CHECK(thrid_row == Matrix_Test[2]);

			one[0][1] = 45;
			first_row = one[0];
			CHECK(first_row.y == 45);

			Matrix3 inv = one.Inversed();
			Matrix3 result = inv * one;
			CHECK(result.Equal(Matrix3::Identity, 0.0001f));
		}

		TEST_CASE("Multiplication")
		{
			Matrix3 one = Matrix_Test;
			Matrix3 two = glmMatrixTest1;

			Matrix3 result = one * two;
			CHECK(result.Equal(glmMatrixTest * glmMatrixTest1, 0.0001f));

			Vector3 vec = Vector3(5, 7, 15);
			glm::vec3 glmVec = glm::vec3(vec.x, vec.y, vec.z);
			glm::mat3 glmMat = glm::mat3(glmMatrixTest) *= glmVec;
			
			result = one *= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));

			CHECK((Matrix_Test * vec) == (glmMatrixTest * glmVec));
		}

		TEST_CASE("Division")
		{
			Matrix3 one = Matrix_Test;
			Matrix3 two = Matrix_Test1;

			glm::mat3 g = glmMatrixTest;

			glm::mat3 glmMat = glmMatrixTest / glmMatrixTest1;
			glm::mat3 glmMat1 = glmMatrixTest1 / glmMatrixTest;

			Matrix3 result = one / two;
			Matrix3 result1 = two / one;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result1.Equal(glmMat1, 0.0001f));

			Vector3 vec = Vector3(5, 7, 15);
			glmMat = glm::mat3(glmMatrixTest) /= glm::vec3(vec.x, vec.y, vec.z);
			result = one /= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
		}

		TEST_CASE("Subtraction")
		{
			Matrix3 one = Matrix_Test;
			Matrix3 two = glmMatrixTest1;

			Matrix3 result = one - two;
			CHECK(result.Equal(glmMatrixTest - glmMatrixTest1, 0.0001f));

			Vector3 vec = Vector3(5, 7, 15);
			glm::vec3 glmVec = glm::vec3(vec.x, vec.y, vec.z);
			glm::mat3 glmMat = glm::mat3(glmMatrixTest) -= glmVec;
			result = one -= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));
		}

		TEST_CASE("Addition")
		{
			Matrix3 one = Matrix_Test;
			Matrix3 two = glmMatrixTest1;

			Matrix3 result = one + two;
			CHECK(result.Equal(glmMatrixTest + glmMatrixTest1, 0.0001f));

			Vector3 vec = Vector3(5, 7, 15);
			glm::vec3 glmVec = glm::vec3(vec.x, vec.y, vec.z);
			glm::mat3 glmMat = glm::mat3(glmMatrixTest) += glmVec;
			result = one += vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));
		}
	}
}
#endif