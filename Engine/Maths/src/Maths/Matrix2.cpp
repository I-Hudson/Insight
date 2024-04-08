#include "Maths/Matrix2.h"
#include "Maths/Vector2.h"

#include "Maths/MathsUtils.h"

#include <cassert>

namespace Insight
{
	namespace Maths
	{
		const Matrix2 Matrix2::Zero     = Matrix2();
		const Matrix2 Matrix2::One      = Matrix2(1.0f, 1.0f, 1.0f, 1.0);
		const Matrix2 Matrix2::Identity = Matrix2(1.0f, 0.0f, 0.0f, 1.0);

		Matrix2::Matrix2()
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f))
#else
			: m_00(0), m_01(0)
			, m_10(0), m_11(0)
#endif
		{ }
		Matrix2::Matrix2(float m00, float m01, float m10, float m11)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  m00, m01, 0.0f, 0.0f
				, m10, m11, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: m_00(m00), m_01(m01)
			, m_10(m10), m_11(m11)
#endif
		{ }
		Matrix2::Matrix2(const Matrix2& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01)
			, m_10(other.m_10), m_11(other.m_11)
#endif
		{ }
		Matrix2::Matrix2(Matrix2&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(other.xmmatrix)
#else
			: m_00(other.m_00), m_01(other.m_01)
			, m_10(other.m_10), m_11(other.m_11)
#endif
		{
			other = Matrix2::Zero;
		}

		Matrix2::Matrix2(const Vector2& v1, const Vector2& v2)
			: v0(v1), v1(v2)
		{ }

#ifdef IS_MATHS_DIRECTX_MATHS
		Matrix2::Matrix2(const DirectX::XMMATRIX& other)
			: xmmatrix(other)
		{ }
		Matrix2::Matrix2(DirectX::XMMATRIX&& other)
			: xmmatrix(other)
		{ }
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
		Matrix2::Matrix2(const glm::mat2& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			/* 
				Because of XMatrix being 64 bytes in size and glm::mat2 only being 16 bytes there is a mismatch in the memory layout
				when setting a Matrix2 from a glm::mat2 and if directx math is being used (in testing, this shouldn't be used in production).
				What happens is the XMatrix is layout out like so:
				00, 01, 02, 03
				10, 11, 12, 13
				20, 21, 22, 23
				30, 31, 32, 33,
				We want to interact with only:
				00, 01,
				10, 11.
				But this means we have 8 bytes 02, 03 which are not used. So when we try and apply our glm::mat2, its second row 
				is set to 02, 03 as they are the next bytes in memory even though we want to skip them, because of this add this code to 
				explicitly set our glm values to the correct elements.
			*/
			: xmmatrix(DirectX::XMMatrixSet(
				other[0].x, other[0].y, 0.0f, 0.0f
				, other[1].x, other[1].y, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: mat2(other)
#endif
		{ }
		Matrix2::Matrix2(glm::mat2&& other)
#ifdef IS_MATHS_DIRECTX_MATHS
			: xmmatrix(DirectX::XMMatrixSet(
				  other[0].x, other[0].y, 0.0f, 0.0f
				, other[1].x, other[1].y, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
#else
			: mat2(other)
#endif
		{ }
#endif

		Matrix2::~Matrix2()
		{ }

		Matrix2 Matrix2::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#elif defined(IS_MATHS_GLM)
			mat2 = glm::inverse(mat2);
#else
			const float determinant = 1.0f / ((m_00 * m_11) - (m_01 * m_10));
			 Matrix2 inverse(
					m_11 * determinant, -m_01 * determinant
				,	-m_10 * determinant, m_00 * determinant);

			*this = inverse;
#endif
			return *this;
		}

		Matrix2 Matrix2::Inversed() const
		{
			return Matrix2(*this).Inverse();
		}

		Matrix2 Matrix2::Transpose()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = DirectX::XMMatrixTranspose(xmmatrix);
#elif defined(IS_MATHS_GLM)
			mat2 = glm::transpose(mat2);
#else
			Matrix2 tran(
				m_00, m_10
				, m_01, m_11);
			*this = tran;
#endif
			return *this;
		}

		Matrix2 Matrix2::Transposed() const
		{
			return Matrix2(*this).Transpose();
		}

		Vector2& Matrix2::operator[](int i)
		{
			switch (i)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			default:
				break;
			}
			assert(false);
			return v0;
		}
		Vector2& Matrix2::operator[](unsigned int i)
		{
			switch (i)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			default:
				break;
			}
			assert(false);
			return v0;
		}
		const Vector2& Matrix2::operator[](int i) const
		{
			return const_cast<Matrix2&>(*this)[i];
		}
		const Vector2& Matrix2::operator[](unsigned int i) const
		{
			return const_cast<Matrix2&>(*this)[i];
		}

		bool Matrix2::operator==(const Matrix2& other) const
		{
			return Equals(m_00, other.m_00) && Equals(m_01, other.m_01)
				&& Equals(m_10, other.m_10) && Equals(m_11, other.m_11);
		}
		bool Matrix2::operator!=(const Matrix2& other) const
		{
			return !(*this == other);
		}

		bool Matrix2::Equal(const Matrix2& other, const float errorRange) const
		{
			return v0.Equal(other.v0, errorRange) && v1.Equal(other.v1, errorRange);
		}

		bool Matrix2::NotEqual(const Matrix2& other, const float errorRange) const
		{
			return !(Equal(other, errorRange));
		}

		Matrix2 Matrix2::operator=(const Matrix2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
#elif defined(IS_MATHS_GLM)
			mat2 = other.mat2;
#else
			m_00 = other.m_00;
			m_01 = other.m_01;
			m_10 = other.m_10;
			m_11 = other.m_11;
#endif
			return *this;
		}

		Vector2 Matrix2::operator*(const Vector2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Vector2(DirectX::XMVector2Transform(other.xmvector, xmmatrix));
#elif defined(IS_MATHS_GLM)
			return mat2 * other.vec2;
#else
			const Vector2 vec_y = Vector2(other.y);
			const Vector2 vec_x = Vector2(other.x);

			Vector2 result;
			result = vec_y * Vector2(v1.x, v1.y) + result;
			result = vec_x * Vector2(v0.x, v0.y) + result;
			return result;
#endif
		}
		Matrix2 Matrix2::operator*(const Matrix2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix2(DirectX::XMMatrixMultiply(other.xmmatrix, xmmatrix));
#elif defined(IS_MATHS_GLM)
			return mat2 * other.mat2;
#else
			return Matrix2(
			(other.m_00 * m_00) + (other.m_01 * m_10), (other.m_00 * m_01) + (other.m_01 * m_11),
			(other.m_10 * m_00) + (other.m_11 * m_10), (other.m_10 * m_01) + (other.m_11 * m_11));
#endif
		}

		Vector2 Matrix2::operator/(const Vector2& other)
		{
			return Inversed() * other;
		}
		Matrix2 Matrix2::operator/(const Matrix2& other)
		{
			return *this * other.Inversed();
		}

		Matrix2 Matrix2::operator-(const Matrix2& other)
		{
			return Matrix2((*this)[0] - other[0], (*this)[1] - other[1]);
		}

		Matrix2 Matrix2::operator+(const Matrix2& other)
		{
			return Matrix2((*this)[0] + other[0], (*this)[1] + other[1]);
		}

		Matrix2 Matrix2::operator*=(const Vector2& other)
		{
			(*this)[0] *= other;
			(*this)[1] *= other;
			return *this;
		}
		Matrix2 Matrix2::operator*=(const Matrix2& other)
		{
			*this = Matrix2(*this) * other;
			return *this;
		}

		Matrix2 Matrix2::operator/=(const Vector2& other)
		{
			(*this)[0] /= other;
			(*this)[1] /= other;
			return *this;
		}
		Matrix2 Matrix2::operator/=(const Matrix2& other)
		{
			*this = Matrix2(*this) / other;
			return *this;
		}

		Matrix2 Matrix2::operator-=(const Vector2& other)
		{
			(*this)[0] -= other;
			(*this)[1] -= other;
			return *this;
		}
		Matrix2 Matrix2::operator-=(const Matrix2& other)
		{
			*this = Matrix2(*this) - other;
			return *this;
		}

		Matrix2 Matrix2::operator+=(const Vector2& other)
		{
			(*this)[0] += other;
			(*this)[1] += other;
			return *this;
		}
		Matrix2 Matrix2::operator+=(const Matrix2& other)
		{
			*this = Matrix2(*this) + other;
			return *this;
		}

		const float* Matrix2::Data() const
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
	TEST_SUITE("Matrix2")
	{
		const float MatrixText_X1 = 4;
		const float MatrixText_Y1 = 7;
		const float MatrixText_X2 = 2;
		const float MatrixText_Y2 = 6;

		const float MatrixText_X3 = 127;
		const float MatrixText_Y3 = 26;
		const float MatrixText_X4 = 785;
		const float MatrixText_Y4 = 1249;

		const glm::mat2 glmMatrixTest(
			MatrixText_X1, MatrixText_Y1,
			MatrixText_X2, MatrixText_Y2);

		const Matrix2 Matrix_Test(
			MatrixText_X1, MatrixText_Y1,
			MatrixText_X2, MatrixText_Y2);

		const glm::mat2 glmMatrixTest1(
			MatrixText_X3, MatrixText_Y3,
			MatrixText_X4, MatrixText_Y4);

		const Matrix2 Matrix_Test1(
			MatrixText_X3, MatrixText_Y3,
			MatrixText_X4, MatrixText_Y4);

		TEST_CASE("Size Of")
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			CHECK(sizeof(Matrix2) == 64);
#else
			CHECK(sizeof(Matrix2) == 16);
#endif
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float x2 = 0.0f;
			float y2 = 0.0f;

			Matrix2 matrix;
			CHECK(matrix[0].x == x1);
			CHECK(matrix[0].y == y1);
			CHECK(matrix[1].x == x2);
			CHECK(matrix[1].y == y2);

			CHECK(matrix.m_00 == x1);
			CHECK(matrix.m_01 == y1);
			CHECK(matrix.m_10 == x2);
			CHECK(matrix.m_11 == y2);

			x1 = 1.0f;
			y1 = 2.0f;
			x2 = 3.0f;
			y2 = 4.0f;
			matrix = Matrix2(x1, y1, x2, y2);
			CHECK(matrix[0].x == x1);
			CHECK(matrix[0].y == y1);
			CHECK(matrix[1].x == x2);
			CHECK(matrix[1].y == y2);

			CHECK(matrix.m_00 == x1);
			CHECK(matrix.m_01 == y1);
			CHECK(matrix.m_10 == x2);
			CHECK(matrix.m_11 == y2);

			Matrix2 matrix2(matrix);
			CHECK(matrix2[0].x == x1);
			CHECK(matrix2[0].y == y1);
			CHECK(matrix2[1].x == x2);
			CHECK(matrix2[1].y == y2);

			CHECK(matrix2.m_00 == x1);
			CHECK(matrix2.m_01 == y1);
			CHECK(matrix2.m_10 == x2);
			CHECK(matrix2.m_11 == y2);
		}

		TEST_CASE("Inverse")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 inv = one.Inversed();
			CHECK(inv.Equal(glm::inverse(glmMatrixTest), 0.0001f));

			//glm::mat2 glmInv = ;
			Matrix2 result = inv * one;
			CHECK(result.Equal(Matrix2::Identity, 0.0001f));
		}

		TEST_CASE("Transpose")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 tran = one.Transposed();
			CHECK(Equals(tran.m_00, 4.0f, 0.0001f));
			CHECK(Equals(tran.m_01, 2.0f, 0.0001f));
			CHECK(Equals(tran.m_10, 7.0f, 0.0001f));
			CHECK(Equals(tran.m_11, 6.0f, 0.0001f));
		
			CHECK(one == glmMatrixTest);
		}

		TEST_CASE("operator[]")
		{
			Matrix2 one = Matrix_Test;
			Vector2 first_row = one[0];
			Vector2 second_row = one[1];
			CHECK(first_row == Vector2(4, 7));
			CHECK(second_row == Vector2(2, 6));

			one[0][1] = 45;
			first_row = one[0];
			CHECK(first_row == Vector2(4, 45));

			Matrix2 inv = one.Inversed();
			Matrix2 result = inv * one;
			CHECK(result.Equal(Matrix2::Identity, 0.0001f));
		}

		TEST_CASE("Multiplication")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 two = glmMatrixTest1;

			Matrix2 result = one * two;
			CHECK(result.Equal(glmMatrixTest * glmMatrixTest1, 0.0001f));

			Vector2 vec = Vector2(5, 7);
			glm::vec2 glmVec = glm::vec2(vec.x, vec.y);
			glm::mat2 glmMat = glm::mat2(glmMatrixTest) *= glmVec;
			result = one *= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));
		}

		TEST_CASE("Division")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 two = Matrix_Test1;

			glm::mat2 g = glmMatrixTest;

			glm::mat2 glmMat = glmMatrixTest / glmMatrixTest1;
			glm::mat2 glmMat1 = glmMatrixTest1 / glmMatrixTest;

			Matrix2 result = one / two;
			Matrix2 result1 = two / one;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result1.Equal(glmMat1, 0.0001f));

			Vector2 vec = Vector2(5, 7);
			glmMat = glm::mat2(glmMatrixTest) /= glm::vec2(vec.x, vec.y);
			result = one /= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
		}

		TEST_CASE("Subtraction")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 two = glmMatrixTest1;

			Matrix2 result = one - two;
			CHECK(result.Equal(glmMatrixTest - glmMatrixTest1, 0.0001f));

			Vector2 vec = Vector2(5, 7);
			glm::vec2 glmVec = glm::vec2(vec.x, vec.y);
			glm::mat2 glmMat = glm::mat2(glmMatrixTest) -= glmVec;
			result = one -= vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));
		}

		TEST_CASE("Addition")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 two = glmMatrixTest1;

			Matrix2 result = one + two;
			CHECK(result.Equal(glmMatrixTest + glmMatrixTest1, 0.0001f));

			Vector2 vec = Vector2(5, 7);
			glm::vec2 glmVec = glm::vec2(vec.x, vec.y);
			glm::mat2 glmMat = glm::mat2(glmMatrixTest) += glmVec;
			result = one += vec;
			CHECK(result.Equal(glmMat, 0.0001f));
			CHECK(result.Equal(one, 0.0001f));
		}
	}
}
#endif