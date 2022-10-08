#include "Maths/Matrix2.h"
#include "Maths/Vector2.h"

#include "Maths/MathsUtils.h"

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
				  1.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 1.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 1.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 1.0f))
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

#ifdef IS_MATHS_DIRECTX_MATHS
		Matrix2::Matrix2(const DirectX::XMMATRIX& other)
			: xmmatrix(other)
		{ }
		Matrix2::Matrix2(DirectX::XMMATRIX&& other)
			: xmmatrix(other)
		{
			DirectX::XMMatrixSet(
			  1.0f, 0.0f, 0.0f, 0.0f
			, 0.0f, 1.0f, 0.0f, 0.0f
			, 0.0f, 0.0f, 1.0f, 0.0f
			, 0.0f, 0.0f, 0.0f, 1.0f);
		}
#endif

		Matrix2::~Matrix2()
		{ }

		Matrix2 Matrix2::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#else
			const float determinant = 1.0f / ((m_00 * m_11) - (m_01 * m_10));
			 Matrix2 inverse(
					m_11 * determinant, -m_01 * determinant
				,	-m_10 * determinant, m_00 * determinant);

			*this = inverse;
#endif
			return *this;
		}
		Matrix2 Matrix2::Transpose()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = DirectX::XMMatrixTranspose(xmmatrix);
#else
			Matrix2 tran(
				m_00, m_10
				, m_01, m_11);
			*this = tran;
#endif
			return *this;
		}

		Vector2& Matrix2::operator[](int i)
		{
			return data[i];
		}
		Vector2& Matrix2::operator[](unsigned int i)
		{
			return data[i];
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

		Matrix2 Matrix2::operator=(const Matrix2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
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
#else
			return Vector2(
					  (m_00 * other.x) + (m_10 * other.y)
					, (m_01 * other.x) + (m_11 * other.y));
#endif
		}
		Matrix2 Matrix2::operator*(const Matrix2& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return Matrix2(DirectX::XMMatrixMultiply(xmmatrix, other.xmmatrix));
#else
			return Matrix2(
			(m_00 * other.m_00) + (m_01 * other.m_10), (m_00 * other.m_01) + (m_01 * other.m_11),
			(m_10 * other.m_00) + (m_11 * other.m_10), (m_10 * other.m_01) + (m_11 * other.m_11));
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
			return Matrix2(
				  m_00 - other.m_00, m_01 - other.m_01
				, m_01 - other.m_01, m_11 - other.m_11);
		}

		Matrix2 Matrix2::operator+(const Matrix2& other)
		{
			return Matrix2(
				  m_00 + other.m_00, m_01 + other.m_01
				, m_01 + other.m_01, m_11 + other.m_11);
		}

		Matrix2 Matrix2::operator*=(const Vector2& other)
		{
			data[0] *= other.x;
			data[1] *= other.y;
			return *this;
		}
		Matrix2 Matrix2::operator*=(const Matrix2& other)
		{
			*this = Matrix2(*this) * other;
			return *this;
		}

		Matrix2 Matrix2::operator/=(const Vector2& other)
		{
			data[0] /= other.x;
			data[1] /= other.y;
			return *this;
		}
		Matrix2 Matrix2::operator/=(const Matrix2& other)
		{
			*this = Matrix2(*this) / other;
			return *this;
		}

		Matrix2 Matrix2::operator-=(const Vector2& other)
		{
			data[0] -= other.x;
			data[1] -= other.y;
			return *this;
		}
		Matrix2 Matrix2::operator-=(const Matrix2& other)
		{
			*this = Matrix2(*this) - other;
			return *this;
		}

		Matrix2 Matrix2::operator+=(const Vector2& other)
		{
			data[0] += other.x;
			data[1] += other.y;
			return *this;
		}
		Matrix2 Matrix2::operator+=(const Matrix2& other)
		{
			*this = Matrix2(*this) + other;
			return *this;
		}

		const float* Matrix2::Data() const
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
	TEST_SUITE("Matrix2")
	{
		const Matrix2 Matrix_Test(
			4, 7,
			2, 6);
		TEST_CASE("Constructors")
		{
			Matrix2 matrix;

		}

		TEST_CASE("Inverse")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 inv = one.Inversed();
			Matrix2 result = one * inv;
			CHECK(result == Matrix2::Identity);
		}

		TEST_CASE("Transpose")
		{
			Matrix2 one = Matrix2(
				4, 7,
				2, 6);
			Matrix2 tran = one.Transpose();
			CHECK(Equals(tran.m_00, 4.0f, 0.0001f));
			CHECK(Equals(tran.m_01, 2.0f, 0.0001f));
			CHECK(Equals(tran.m_10, 7.0f, 0.0001f));
			CHECK(Equals(tran.m_11, 6.0f, 0.0001f));
		}

		TEST_CASE("operator[]")
		{
			Matrix2 one = Matrix2(
				4, 7,
				2, 6);
			Vector2 first_row = one[0];
			Vector2 second_row = one[1];
			CHECK(first_row == Vector2(4, 7));
			CHECK(second_row == Vector2(2, 6));

			one[0][1] = 45;
			first_row = one[0];
			CHECK(first_row == Vector2(4, 45));

			Matrix2 inv = one.Inversed();
			Matrix2 result = one * inv;
			CHECK(result == Matrix2::Identity);
		}

		TEST_CASE("Multiplcation")
		{
			Matrix2 one = Matrix2(5, 10,
								  25,9);
			Matrix2 two = Matrix2(2, 5,
								  3, 4);

			Matrix2 result = one * two;
			CHECK(Equals(result.m_00, 40.0f, 0.0001f));
			CHECK(Equals(result.m_01, 65.0f, 0.0001f));
			CHECK(Equals(result.m_10, 77.0f, 0.0001f));
			CHECK(Equals(result.m_11, 161.0f, 0.0001f));

			Vector2 vec = Vector2(5, 7);
			result = one *= vec;
			CHECK(Equals(result.m_00, 25.0f, 0.0001f));
			CHECK(Equals(result.m_01, 50.0f, 0.0001f));
			CHECK(Equals(result.m_10, 175.0f, 0.0001f));
			CHECK(Equals(result.m_11, 63.0f, 0.0001f));
			CHECK(result == one);
		}

		TEST_CASE("Divition")
		{
			Matrix2 one = Matrix_Test;
			Matrix2 two = Matrix2(2, 5,
				3, 4);

			Matrix2 result = one / two;
			CHECK(Equals(result.m_00, 0.71429f, 0.001f));
			CHECK(Equals(result.m_01, 0.85714f, 0.001f));
			CHECK(Equals(result.m_10, 1.42857f, 0.001f));
			CHECK(Equals(result.m_11, -0.28571f, 0.001f));

			Vector2 vec = Vector2(5, 7);
			result = one /= vec;
			CHECK(Equals(result.m_00, 0.8000f, 0.001f));
			CHECK(Equals(result.m_01, 1.3999f, 0.001f));
			CHECK(Equals(result.m_10, 0.2857f, 0.001f));
			CHECK(Equals(result.m_11, 0.8571f, 0.001f));
			CHECK(result == one);
		}

		TEST_CASE("Subtraction")
		{
			Matrix2 one = Matrix2(5, 10,
								 25, 9);
			Vector2 vec = Vector2(5, 7);
			Matrix2 result = one -= vec;
			CHECK(Equals(result.m_00, 0.0f, 0.001f));
			CHECK(Equals(result.m_01, 5.0f, 0.001f));
			CHECK(Equals(result.m_10, 18.0f, 0.001f));
			CHECK(Equals(result.m_11, 2.0f, 0.001f));
			CHECK(result == one);
		}
	}
}
#endif