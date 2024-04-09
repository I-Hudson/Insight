#include "Maths/Matrix4.h"
#include "Maths/Vector4.h"

#include "Maths/MathsUtils.h"

#include <cassert>

#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#endif

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
				  0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f
				, 0.0f, 0.0f, 0.0f, 0.0f))
#else
			: m_00(0.0f), m_01(0.0f), m_02(0.0f), m_03(0.0f)
			, m_10(0.0f), m_11(0.0f), m_12(0.0f), m_13(0.0f)
			, m_20(0.0f), m_21(0.0f), m_22(0.0f), m_23(0.0f)
			, m_30(0.0f), m_31(0.0f), m_32(0.0f), m_33(0.0f)
#endif
		{ }
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
		{ }
		Matrix4::Matrix4(const Vector4 v0, const Vector4 v1, const Vector4 v2, const Vector4 v3)
			: v0(v0)
			, v1(v1)
			, v2(v2)
			, v3(v3)
		{ }
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
		{ }

#ifdef IS_MATHS_DIRECTX_MATHS
		Matrix4::Matrix4(const DirectX::XMMATRIX& other)
			: xmmatrix(other)
		{ }
		Matrix4::Matrix4(DirectX::XMMATRIX&& other)
			: xmmatrix(other)
		{ }
		Matrix4::Matrix4(const DirectX::XMFLOAT4X4 & other)
			: xmmatrix(DirectX::XMMatrixSet(
				  other._11, other._12, other._13, other._14
				, other._21, other._22, other._23, other._24
				, other._31, other._32, other._33, other._34
				, other._41, other._42, other._43, other._44))
		{ }
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)

		Matrix4::Matrix4(const glm::mat4& other)
			: mat4(other)
		{ }
		Matrix4::Matrix4(glm::mat4&& other)
			: mat4(other)
		{ }
#endif

		Matrix4::~Matrix4()
		{ }

		Matrix4 Matrix4::Inverse()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR determinant;
			xmmatrix = DirectX::XMMatrixInverse(&determinant, xmmatrix);
#elif defined(IS_MATHS_GLM)
			mat4 = glm::inverse(mat4);
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
#elif defined(IS_MATHS_GLM)
			mat4 = glm::transpose(mat4);
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
		Matrix4 Matrix4::Translate(const Vector4 vector)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(vector.xmvector), xmmatrix);
#elif defined(IS_MATHS_GLM)
			return glm::translate(mat4, glm::vec3(vector.vec4));
#else
			assert(false);
			return Matrix4();
#endif
		}

		Matrix4 Matrix4::CreatePerspective(const float fovy, const float aspect, const float zNear, const float zFar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixPerspectiveFovRH(fovy, aspect, zNear, zFar);
#elif defined(IS_MATHS_GLM)
			return glm::perspective(fovy, aspect, zNear, zFar);
#else
			Matrix4 Result = Matrix4::Identity;
			assert(false);
			return Result;
#endif
		}

		Matrix4 Matrix4::CreateOrthographic(const float left, const float right, const float bottom, const float top)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixOrthographicRH(abs(left - right), abs(bottom - top), 0, std::numeric_limits<float>::max());
#elif defined(IS_MATHS_GLM)
			return glm::ortho(left, right, bottom, top);
#else
			Matrix4 Result = Matrix4::Identity;
			assert(false);
			return Result;
#endif
		}
		Matrix4 Matrix4::CreateOrthographic(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixOrthographicRH(abs(left - right), abs(bottom - top), zNear, zFar);
#elif defined(IS_MATHS_GLM)
			return glm::ortho(left, right, bottom, top, zNear, zFar);
#else
			Matrix4 Result = Matrix4::Identity;
			assert(false);
			return Result;
#endif
		}

		Matrix4 Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixLookAtRH(eye.xmvector, center.xmvector, up.xmvector);
#elif defined(IS_MATHS_GLM)
			return glm::lookAt(eye.vec3, center.vec3, up.vec3);
#else
			Matrix4 Result = Matrix4::Identity;
			assert(false);
			return Result;
#endif
		}

		Vector4& Matrix4::operator[](int i)
		{
			switch (i)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
			case 3:
				return v3;
			default:
				break;
			}
			assert(false);
			return v0;
		}
		const Vector4& Matrix4::operator[](int i) const
		{
			switch (i)
			{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
			case 3:
				return v3;
			default:
				break;
			}
			assert(false);
			return v0;
		}

		bool Matrix4::operator==(const Matrix4& other) const
		{
			return Equals(m_00, other.m_00, 0.1f) && Equals(m_01, other.m_01, 0.1f) && Equals(m_02, other.m_02, 0.1f) && Equals(m_03, other.m_03, 0.1f)
				&& Equals(m_10, other.m_10, 0.1f) && Equals(m_11, other.m_11, 0.1f) && Equals(m_12, other.m_12, 0.1f) && Equals(m_13, other.m_13, 0.1f)
				&& Equals(m_20, other.m_20, 0.1f) && Equals(m_21, other.m_21, 0.1f) && Equals(m_22, other.m_22, 0.1f) && Equals(m_23, other.m_23, 0.1f)
				&& Equals(m_30, other.m_30, 0.1f) && Equals(m_31, other.m_31, 0.1f) && Equals(m_32, other.m_32, 0.1f) && Equals(m_33, other.m_33, 0.1f);
		}
		bool Matrix4::operator!=(const Matrix4& other) const
		{
			return !(*this == other);
		}

		bool Matrix4::Equal(const Matrix4& other, const float errorRange) const
		{
			bool b = v0.Equal(other.v0, errorRange);
			bool b1 = v1.Equal(other.v1, errorRange);
			bool b2 = v2.Equal(other.v2, errorRange);
			bool b3 = v3.Equal(other.v3, errorRange);
			return v0.Equal(other.v0, errorRange) && v1.Equal(other.v1, errorRange) && v2.Equal(other.v2, errorRange) && v3.Equal(other.v3, errorRange);
		}

		bool Matrix4::NotEqual(const Matrix4& other, const float errorRange) const
		{
			return !Equal(other, errorRange);
		}

		Matrix4 Matrix4::operator=(const Matrix4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			xmmatrix = other.xmmatrix;
#elif defined(IS_MATHS_GLM)
			mat4 = other.mat4;
#else
			m_00 = other.m_00; m_01 = other.m_01; m_02 = other.m_02; m_03 = other.m_03;
			m_10 = other.m_10; m_11 = other.m_11; m_12 = other.m_12; m_13 = other.m_13;
			m_20 = other.m_20; m_21 = other.m_21; m_22 = other.m_22; m_23 = other.m_23;
			m_30 = other.m_30; m_31 = other.m_31; m_32 = other.m_32; m_33 = other.m_33;
#endif
			return *this;
		}

		Matrix4 Matrix4::operator*(const Matrix4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMMatrixMultiply(other.xmmatrix, xmmatrix);
#elif defined(IS_MATHS_GLM)
			return mat4 * other.mat4;
#else
			Matrix4 m;

			float x = other[0][0];
			float y = other[0][1];
			float z = other[0][2];
			float w = other[0][3];
			m[0][0] = (x * m_00) + (y * m_10) + (z * m_20) + (w * m_30);
			m[0][1] = (x * m_01) + (y * m_11) + (z * m_21) + (w * m_31);
			m[0][2] = (x * m_02) + (y * m_12) + (z * m_22) + (w * m_32);
			m[0][3] = (x * m_03) + (y * m_13) + (z * m_23) + (w * m_33);

			x = other[1][0];
			y = other[1][1];
			z = other[1][2];
			w = other[1][3];
			m[1][0] = (x * m_00) + (y * m_10) + (z * m_20) + (w * m_30);
			m[1][1] = (x * m_01) + (y * m_11) + (z * m_21) + (w * m_31);
			m[1][2] = (x * m_02) + (y * m_12) + (z * m_22) + (w * m_32);
			m[1][3] = (x * m_03) + (y * m_13) + (z * m_23) + (w * m_33);

			x = other[2][0];
			y = other[2][1];
			z = other[2][2];
			w = other[2][3];
			m[2][0] = (x * m_00) + (y * m_10) + (z * m_20) + (w * m_30);
			m[2][1] = (x * m_01) + (y * m_11) + (z * m_21) + (w * m_31);
			m[2][2] = (x * m_02) + (y * m_12) + (z * m_22) + (w * m_32);
			m[2][3] = (x * m_03) + (y * m_13) + (z * m_23) + (w * m_33);

			x = other[3][0];
			y = other[3][1];
			z = other[3][2];
			w = other[3][3];
			m[3][0] = (x * m_00) + (y * m_10) + (z * m_20) + (w * m_30);
			m[3][1] = (x * m_01) + (y * m_11) + (z * m_21) + (w * m_31);
			m[3][2] = (x * m_02) + (y * m_12) + (z * m_22) + (w * m_32);
			m[3][3] = (x * m_03) + (y * m_13) + (z * m_23) + (w * m_33);

			return m;
#endif
		}
		Vector4 Matrix4::operator*(const Vector4& other) const
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVector4Transform(other.xmvector, xmmatrix);
#elif defined(IS_MATHS_GLM)
			return mat4 * other.vec4;
#else
			const float x = (m_00 * other.x) + (m_10 * other.y) + (m_20 * other.z) + (m_30 * other.w);
			const float y = (m_01 * other.x) + (m_11 * other.y) + (m_21 * other.z) + (m_31 * other.w);
			const float z = (m_02 * other.x) + (m_12 * other.y) + (m_22 * other.z) + (m_32 * other.w);
			const float w = (m_03 * other.x) + (m_13 * other.y) + (m_23 * other.z) + (m_33 * other.w);
			return Vector4(x, y, z, w);
#endif
		}

		Matrix4 Matrix4::operator/(const Matrix4& other) const
		{
			return *this * other.Inversed();
		}
		Vector4 Matrix4::operator/(const Vector4& other) const
		{
			return Inversed() * other;
		}

		Matrix4 Matrix4::operator-(const Matrix4& other) const
		{
#if defined(IS_MATHS_GLM)
			return mat4 - other.mat4;
#else
			return Matrix4(
				v0 - other.v0,
				v1 - other.v1,
				v2 - other.v2,
				v3 - other.v3);
#endif
		}

		Matrix4 Matrix4::operator+(const Matrix4& other) const
		{
#if defined(IS_MATHS_GLM)
			return mat4 + other.mat4;
#else
			return Matrix4(
				v0 + other.v0,
				v1 + other.v1,
				v2 + other.v2,
				v3 + other.v3);
#endif
		}

		Matrix4 Matrix4::operator*=(const Matrix4& other)
		{
			*this = Matrix4(*this) * other;
			return *this;
		}
		Matrix4 Matrix4::operator*=(const Vector4& other)
		{
			v0 *= other;
			v1 *= other;
			v2 *= other;
			v3 *= other;
			return *this;
		}

		Matrix4 Matrix4::operator/=(const Matrix4& other)
		{
			*this = Matrix4(*this) / other;
			return *this;
		}
		Matrix4 Matrix4::operator/=(const Vector4& other)
		{	
			v0 /= other;
			v1 /= other;
			v2 /= other;
			v3 /= other;
			return *this;
		}

		Matrix4 Matrix4::operator-=(const Matrix4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			//return Vector4(DirectX::XMVector4Transform(other.xmvector, xmmatrix));
#elif defined(IS_MATHS_GLM)
			mat4 -= other.mat4;
#else
			*this = Matrix4(*this) - other;
#endif
			return *this;
		}
		Matrix4 Matrix4::operator-=(const Vector4& other)
		{
			v0 -= other;
			v1 -= other;
			v2 -= other;
			v3 -= other;
			return *this;
		}

		Matrix4 Matrix4::operator+=(const Matrix4& other)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			//xmmatrix = Vector4(DirectX::XMVector4Transform(other.xmvector, xmmatrix));
#elif defined(IS_MATHS_GLM)
			mat4 += other.mat4;
#else
			*this = Matrix4(*this) + other;
#endif
			return *this;
		}
		Matrix4 Matrix4::operator+=(const Vector4& other)
		{
			v0 += other;
			v1 += other;
			v2 += other;
			v3 += other;
			return *this;
		}

		const float* Matrix4::Data() const
		{
			return v0.Data();
		}
	}
}

#ifdef IS_TESTING
#include "doctest.h"
namespace test
{
	using namespace Insight::Maths;
	TEST_SUITE("Matrix4")
	{
		const float MatrixText_X1 = 4;
		const float MatrixText_Y1 = 7;
		const float MatrixText_Z1 = 3;
		const float MatrixText_W1 = 0;

		const float MatrixText_X2 = 2;
		const float MatrixText_Y2 = 6;
		const float MatrixText_Z2 = 9;
		const float MatrixText_W2 = 0;

		const float MatrixText_X3 = 127;
		const float MatrixText_Y3 = 26;
		const float MatrixText_Z3 = 87;
		const float MatrixText_W3 = 0;

		const float MatrixText_X4 = 785;
		const float MatrixText_Y4 = 1249;
		const float MatrixText_Z4 = 8613;
		const float MatrixText_W4 = 1;

		const float MatrixText_X5 = 578;
		const float MatrixText_Y5 = 231;
		const float MatrixText_Z5 = 85;
		const float MatrixText_W5 = 0;

		const float MatrixText_X6 = 983;
		const float MatrixText_Y6 = 849;
		const float MatrixText_Z6 = 826;
		const float MatrixText_W6 = 0;

		const float MatrixText_X7 = 134;
		const float MatrixText_Y7 = 487;
		const float MatrixText_Z7 = 813;
		const float MatrixText_W7 = 0;

		const float MatrixText_X8 = 218;
		const float MatrixText_Y8 = 26;
		const float MatrixText_Z8 = 34;
		const float MatrixText_W8 = 1;

		const glm::mat4 glmMatrixTest(
			MatrixText_X1, MatrixText_Y1, MatrixText_Z1, MatrixText_W1,
			MatrixText_X2, MatrixText_Y2, MatrixText_Z2, MatrixText_W2,
			MatrixText_X3, MatrixText_Y3, MatrixText_Z3, MatrixText_W3,
			MatrixText_X4, MatrixText_Y4, MatrixText_Z4, MatrixText_W4);

		const Matrix4 Matrix_Test(
			MatrixText_X1, MatrixText_Y1, MatrixText_Z1, MatrixText_W1,
			MatrixText_X2, MatrixText_Y2, MatrixText_Z2, MatrixText_W2,
			MatrixText_X3, MatrixText_Y3, MatrixText_Z3, MatrixText_W3,
			MatrixText_X4, MatrixText_Y4, MatrixText_Z4, MatrixText_W4);


		const glm::mat4 glmMatrixTest1(
			MatrixText_X5, MatrixText_Y5, MatrixText_Z5, MatrixText_W5,
			MatrixText_X6, MatrixText_Y6, MatrixText_Z6, MatrixText_W6,
			MatrixText_X7, MatrixText_Y7, MatrixText_Z7, MatrixText_W7,
			MatrixText_X8, MatrixText_Y8, MatrixText_Z8, MatrixText_W8);

		const Matrix4 Matrix_Test1(
			MatrixText_X5, MatrixText_Y5, MatrixText_Z5, MatrixText_W5,
			MatrixText_X6, MatrixText_Y6, MatrixText_Z6, MatrixText_W6,
			MatrixText_X7, MatrixText_Y7, MatrixText_Z7, MatrixText_W7,
			MatrixText_X8, MatrixText_Y8, MatrixText_Z8, MatrixText_W8);

		TEST_CASE("Size Of")
		{
			CHECK(sizeof(Matrix4) == 64);
		}

		TEST_CASE("Constructors")
		{
			float x1 = 0.0f;
			float y1 = 0.0f;
			float z1 = 0.0f;
			float w1 = 0.0f;

			float x2 = 0.0f;
			float y2 = 0.0f;
			float z2 = 0.0f;
			float w2 = 0.0f;

			float x3 = 0.0f;
			float y3 = 0.0f;
			float z3 = 0.0f;
			float w3 = 0.0f;

			float x4 = 0.0f;
			float y4 = 0.0f;
			float z4 = 0.0f;
			float w4 = 0.0f;

			Matrix4 matrix;
			{
				CHECK(matrix[0].x == x1);
				CHECK(matrix[0].y == y1);
				CHECK(matrix[0].z == z1);
				CHECK(matrix[0].w == w1);

				CHECK(matrix[1].x == x2);
				CHECK(matrix[1].y == y2);
				CHECK(matrix[1].z == z2);
				CHECK(matrix[1].w == w2);

				CHECK(matrix[2].x == x3);
				CHECK(matrix[2].y == y3);
				CHECK(matrix[2].z == z3);
				CHECK(matrix[2].w == w3);

				CHECK(matrix[3].x == x4);
				CHECK(matrix[3].y == y4);
				CHECK(matrix[3].z == z4);
				CHECK(matrix[3].w == w4);
			}

			{
				CHECK(matrix.m_00 == x1);
				CHECK(matrix.m_01 == y1);
				CHECK(matrix.m_02 == z1);
				CHECK(matrix.m_03 == w1);

				CHECK(matrix.m_10 == x2);
				CHECK(matrix.m_11 == y2);
				CHECK(matrix.m_12 == z2);
				CHECK(matrix.m_13 == w2);

				CHECK(matrix.m_20 == x3);
				CHECK(matrix.m_21 == y3);
				CHECK(matrix.m_22 == z3);
				CHECK(matrix.m_23 == w3);

				CHECK(matrix.m_30 == x4);
				CHECK(matrix.m_31 == y4);
				CHECK(matrix.m_32 == z4);
				CHECK(matrix.m_33 == w4);
			}

			x1 = 1.0f;
			y1 = 2.0f;
			z1 = 3.0f;
			w1 = 4.0f;

			x2 = 5.0f;
			y2 = 6.0f;
			z2 = 7.0f;
			w2 = 8.0f;

			matrix = Matrix4(
				x1, y1, z1, w1, 
				x2, y2, z2, w2,
				x3, y3, z3, w3,
				x4, y4, z4, w4);
			{
				CHECK(matrix[0].x == x1);
				CHECK(matrix[0].y == y1);
				CHECK(matrix[0].z == z1);
				CHECK(matrix[0].w == w1);

				CHECK(matrix[1].x == x2);
				CHECK(matrix[1].y == y2);
				CHECK(matrix[1].z == z2);
				CHECK(matrix[1].w == w2);

				CHECK(matrix[2].x == x3);
				CHECK(matrix[2].y == y3);
				CHECK(matrix[2].z == z3);
				CHECK(matrix[2].w == w3);

				CHECK(matrix[3].x == x4);
				CHECK(matrix[3].y == y4);
				CHECK(matrix[3].z == z4);
				CHECK(matrix[3].w == w4);
			}

			{
				CHECK(matrix.m_00 == x1);
				CHECK(matrix.m_01 == y1);
				CHECK(matrix.m_02 == z1);
				CHECK(matrix.m_03 == w1);

				CHECK(matrix.m_10 == x2);
				CHECK(matrix.m_11 == y2);
				CHECK(matrix.m_12 == z2);
				CHECK(matrix.m_13 == w2);

				CHECK(matrix.m_20 == x3);
				CHECK(matrix.m_21 == y3);
				CHECK(matrix.m_22 == z3);
				CHECK(matrix.m_23 == w3);

				CHECK(matrix.m_30 == x4);
				CHECK(matrix.m_31 == y4);
				CHECK(matrix.m_32 == z4);
				CHECK(matrix.m_33 == w4);
			}

			Matrix4 matrix2(matrix);
			{
				CHECK(matrix2[0].x == x1);
				CHECK(matrix2[0].y == y1);
				CHECK(matrix2[0].z == z1);
				CHECK(matrix2[0].w == w1);

				CHECK(matrix2[1].x == x2);
				CHECK(matrix2[1].y == y2);
				CHECK(matrix2[1].z == z2);
				CHECK(matrix2[1].w == w2);

				CHECK(matrix2[2].x == x3);
				CHECK(matrix2[2].y == y3);
				CHECK(matrix2[2].z == z3);
				CHECK(matrix2[2].w == w3);

				CHECK(matrix2[3].x == x4);
				CHECK(matrix2[3].y == y4);
				CHECK(matrix2[3].z == z4);
				CHECK(matrix2[3].w == w4);
			}

			{
				CHECK(matrix2.m_00 == x1);
				CHECK(matrix2.m_01 == y1);
				CHECK(matrix2.m_02 == z1);
				CHECK(matrix2.m_03 == w1);

				CHECK(matrix2.m_10 == x2);
				CHECK(matrix2.m_11 == y2);
				CHECK(matrix2.m_12 == z2);
				CHECK(matrix2.m_13 == w2);

				CHECK(matrix2.m_20 == x3);
				CHECK(matrix2.m_21 == y3);
				CHECK(matrix2.m_22 == z3);
				CHECK(matrix2.m_23 == w3);

				CHECK(matrix2.m_30 == x4);
				CHECK(matrix2.m_31 == y4);
				CHECK(matrix2.m_32 == z4);
				CHECK(matrix2.m_33 == w4);
			}
		}

		TEST_CASE("Inverse")
		{
			Matrix4 one = Matrix_Test1;
			Matrix4 inv = one.Inversed();
			glm::mat4 glmInv = glm::inverse(glmMatrixTest1);
			CHECK(inv.Equal(glmInv, 0.1f));

			Matrix4 result = inv * one;
			CHECK(result.Equal(Matrix4::Identity, 0.1f));
		}

		TEST_CASE("Transpose")
		{
			CHECK(Matrix_Test.Transposed() == glm::transpose(glmMatrixTest));
			Matrix4 mat = Matrix_Test;
			mat.Transpose();
			CHECK(mat == glm::transpose(glmMatrixTest));
		}

		TEST_CASE("operator[]")
		{
			Matrix4 one = Matrix_Test;
			Vector4 first_row = one[0];
			Vector4 second_row = one[1];
			Vector4 thrid_row = one[2];
			Vector4 four_row = one[3];

			CHECK(first_row == Matrix_Test[0]);
			CHECK(second_row == Matrix_Test[1]);
			CHECK(thrid_row == Matrix_Test[2]);
			CHECK(four_row == Matrix_Test[3]);

			one[0][1] = 45;
			first_row = one[0];
			CHECK(first_row.y == 45);

			Matrix4 inv = one.Inversed();
			Matrix4 result = inv * one;
			CHECK(result.Equal(Matrix4::Identity, 0.1f));
		}

		TEST_CASE("Multiplication")
		{
			Matrix4 one = Matrix_Test;
			Matrix4 two = Matrix_Test1;

			Matrix4 result = one * two;
			CHECK(result.Equal(glmMatrixTest * glmMatrixTest1, 0.1f));

			Vector4 vec = Vector4(5, 7, 15, 4);
			glm::vec4 glmVec = glm::vec4(vec.x, vec.y, vec.z, vec.w);
			glm::mat4 glmMat = glm::mat4(glmMatrixTest) *= glmVec;

			result = one *= vec;
			CHECK(result.Equal(glmMat, 0.1f));
			CHECK(result.Equal(one, 0.1f));

			CHECK((Matrix_Test * vec) == (glmMatrixTest * glmVec));
		}

		TEST_CASE("Division")
		{
			Matrix4 one = Matrix_Test;
			Matrix4 two = Matrix_Test1;

			glm::mat4 g = glmMatrixTest;

			glm::mat4 glmMat = glmMatrixTest / glmMatrixTest1;
			glm::mat4 glmMat1 = glmMatrixTest1 / glmMatrixTest;

			Matrix4 result = one / two;
			Matrix4 result1 = two / one;
			CHECK(result.Equal(glmMat, 0.1f));
			CHECK(result1.Equal(glmMat1, 0.1f));

			Vector4 vec = Vector4(5, 7, 15, 4);
			glmMat = glm::mat4(glmMatrixTest) /= glm::vec4(vec.x, vec.y, vec.z, vec.w);
			result = one /= vec;
			CHECK(result.Equal(glmMat, 0.1f));
		}

		TEST_CASE("Subtraction")
		{
			Matrix4 one = Matrix_Test;
			Matrix4 two = glmMatrixTest1;

			Matrix4 result = one - two;
			CHECK(result.Equal(glmMatrixTest - glmMatrixTest1, 0.1f));

			Vector4 vec = Vector4(5, 7, 15, 4);
			glm::vec4 glmVec = glm::vec4(vec.x, vec.y, vec.z, vec.w);
			glm::mat4 glmMat = glm::mat4(glmMatrixTest) -= glmVec;
			result = one -= vec;
			CHECK(result.Equal(glmMat, 0.1f));
			CHECK(result.Equal(one, 0.1f));
		}

		TEST_CASE("Addition")
		{
			Matrix4 one = Matrix_Test;
			Matrix4 two = Matrix_Test1;

			Matrix4 result = one + two;
			CHECK(result.Equal(glmMatrixTest + glmMatrixTest1, 0.1f));

			Vector4 vec = Vector4(5, 7, 15, 4);
			glm::vec4 glmVec = glm::vec4(vec.x, vec.y, vec.z, vec.w);
			glm::mat4 glmMat = glm::mat4(glmMatrixTest) += glmVec;
			result = one += vec;
			CHECK(result.Equal(glmMat, 0.1f));
			CHECK(result.Equal(one, 0.1f));
		}
	}
}
#endif