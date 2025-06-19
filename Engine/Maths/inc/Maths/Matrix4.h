#pragma once

#include "Maths/Defines.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"
#include "Quaternion.h"

#include <Reflect/Core/Defines.h>

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif

#include <cmath>

namespace Insight
{
	namespace Maths
	{

		/// @brief Four by Four matrix (4 by 4). Row major.
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
		class IS_MATHS Matrix4
		{
		public:
			Matrix4();
			Matrix4(float m00, float m01, float m02, float m_03,
				float m10, float m11, float m12, float m_13,
				float m20, float m21, float m22, float m_23,
				float m30, float m31, float m32, float m_33);
			
			Matrix4(const Vector4 v0, const Vector4 v1, const Vector4 v2, const Vector4 v3);

			Matrix4(const Quaternion& q);

			Matrix4(const Matrix4& other);
			Matrix4(Matrix4&& other);

#ifdef IS_MATHS_DIRECTX_MATHS
			Matrix4(const DirectX::XMMATRIX& other);
			Matrix4(DirectX::XMMATRIX&& other);
			Matrix4(const DirectX::XMFLOAT4X4& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Matrix4(const glm::mat4& other);
			Matrix4(glm::mat4&& other);
#endif
			~Matrix4();

			/// @brief Inverse this matrix and return a copy (Modifies this matrix).
			/// @return Matrix4
			Matrix4& Inverse();
			/// @brief Return the inverse of this matrix.
			/// @return Matrix4
			Matrix4 Inversed() const { return Matrix4(*this).Inverse(); }
			/// @brief Transpose this matrix and return a copy (Modifies this matrix).
			/// @return Matrix4
			Matrix4& Transpose();
			/// @brief Return the Transpose of this matrix.
			/// @return Matrix4
			Matrix4 Transposed() const { return Matrix4(*this).Transpose(); }

			Matrix4& Translate(const Vector4 vector);
			Matrix4 Translated(const Vector4 vector) const { return Matrix4(*this).Translate(vector); }

			Matrix4& Rotate(const Quaternion& quat);
			Matrix4 Rotated(const Quaternion& quat) const { return Matrix4(*this).Rotate(quat); }

			Matrix4& Scale(const Vector4& vector);
			Matrix4 Scaled(const Vector4& vector) const { return Matrix4(*this).Scale(vector); }

			void Decompose(Vector4& position, Quaternion& rotation, Vector4& scale) const;

			static Matrix4 CreatePerspective(const float fovy, const float aspect, const float zNear, const float zFar);
			static Matrix4 CreateOrthographic(const float left, const float right, const float bottom, const float top);
			static Matrix4 CreateOrthographic(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar);
			static Matrix4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

			Vector4& operator[](int i);
			const Vector4& operator[](int i) const;

			bool operator==(const Matrix4& other) const;
			bool operator!=(const Matrix4& other) const;

			bool Equal(const Matrix4& other, const float errorRange) const;
			bool NotEqual(const Matrix4& other, const float errorRange) const;

			Matrix4& operator=(const Matrix4& other);

			Matrix4 operator*(const Matrix4& other) const;
			Vector4 operator*(const Vector4& other) const;

			Matrix4 operator/(const Matrix4& other) const;
			Vector4 operator/(const Vector4& other) const;

			Matrix4 operator-(const Matrix4& other) const;

			Matrix4 operator+(const Matrix4& other) const;

			Matrix4& operator*=(const Matrix4& other);
			/// @brief Multiple each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4& operator*=(const Vector4& other);

			Matrix4& operator/=(const Matrix4& other);
			/// @brief Divide each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4& operator/=(const Vector4& other);

			Matrix4& operator-=(const Matrix4& other);
			/// @brief Subtract each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4& operator-=(const Vector4& other);

			Matrix4& operator+=(const Matrix4& other);
			/// @brief Add each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4& operator+=(const Vector4& other);

		private:
			float GetDeterminant() const;

		public:

			const float* Data() const;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				// Vector4 is off size XMVECTOR which is a vector 4.
				struct { DirectX::XMMATRIX xmmatrix; };
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
				struct { glm::mat4 mat4; };
#endif
				struct
				{
					Vector4 v0;
					Vector4 v1;
					Vector4 v2;
					Vector4 v3;
				};
				struct
				{
					float
						m_00, m_01, m_02, m_03,
						m_10, m_11, m_12, m_13,
						m_20, m_21, m_22, m_23,
						m_30, m_31, m_32, m_33;
				};
			};
			static const Matrix4 Zero;
			static const Matrix4 One;
			static const Matrix4 Identity;
		};

		IS_MATHS Matrix4 AxisAngleMatrix(const Vector3& axis, const float angle);

		template<typename T>
		Vec<3, T> Vector3TransformCoord(const Vec<4, T> v, const Matrix4 m)
		{
			const Vec<4, T> z = Vec<4, T>(v.z);
			const Vec<4, T> y = Vec<4, T>(v.y);
			const Vec<4, T> x = Vec<4, T>(v.x);

			Vec<4, T> result = (z * m[2]) + m[3];
			result = (y * m[1]) + result;
			result = (x * m[0]) + result;

			const Vec<4, T> w = Vec<4, T>(result.w);
			result = result / w;

#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR dxVector = DirectX::XMVector3TransformCoord(v.xmvector, m.xmmatrix);
			assert(dxVector.m128_f32[0] == result[0]);
			assert(dxVector.m128_f32[1] == result[1]);
			assert(dxVector.m128_f32[2] == result[2]);
			assert(dxVector.m128_f32[3] == result[3]);
#endif
			return Maths::Vector3(result);
		}

		template<typename T>
		Vec<3, T> Vector3Transform(const Vec<3, T> v, const Matrix4 m)
		{
			Vec<4, T> z = Vec<3, T>(v.z);
			Vec<4, T> y = Vec<3, T>(v.y);
			Vec<4, T> x = Vec<3, T>(v.x);

			Vec<3, T> result = (z * m[2]) + m[3];
			result = Maths::Vec<3, T>((y * m[1])) + result;
			result = Maths::Vec<3, T>((x * m[0])) + result;

#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR dxVector = DirectX::XMVector3Transform(v.xmvector, m.xmmatrix);
			assert(dxVector.m128_f32[0] == result[0]);
			assert(dxVector.m128_f32[1] == result[1]);
			assert(dxVector.m128_f32[2] == result[2]);
#endif
			return result;
		}

		template<typename T>
		Vec<3, T> Vector3TransformFromNormal(const Maths::Matrix4 m, const Vec<3, T> vec)
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return DirectX::XMVector3TransformNormal(vec.xmvector, m.xmmatrix);
#else
			XMVECTOR Z = XMVectorSplatZ(V);
			XMVECTOR Y = XMVectorSplatY(V);
			XMVECTOR X = XMVectorSplatX(V);

			XMVECTOR Result = XMVectorMultiply(Z, M.r[2]);
			Result = XMVectorMultiplyAdd(Y, M.r[1], Result);
			Result = XMVectorMultiplyAdd(X, M.r[0], Result);

			return Result;
#endif
		}

		template<typename T>
		Vec<4, T> Vector4TransofrmCoord(const Vec<4, T> v, const Matrix4 m)
		{
			Vec<4, T> z = Vec<4, T>(v.z);
			Vec<4, T> y = Vec<4, T>(v.y);
			Vec<4, T> x = Vec<4, T>(v.x);

			Vec<4, T> result = (z * m[2]) + m[3];
			result = (y * m[1]) + result;
			result = (x * m[0]) + result;

			const Vec<4, T> w = Vec<4, T>(result.w);
			result = result / w;

#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR dxVector = DirectX::XMVector3TransformCoord(v.xmvector, m.xmmatrix);
			assert(dxVector.m128_f32[0] == result[0]);
			assert(dxVector.m128_f32[1] == result[1]);
			assert(dxVector.m128_f32[2] == result[2]);
			assert(dxVector.m128_f32[3] == result[3]);
#endif
			return result;
		}

		template<typename T>
		Vec<4, T> Vector4Transform(const Vec<4, T> v, const Matrix4 m)
		{
			Vec<4, T> z = Vec<4, T>(v.z);
			Vec<4, T> y = Vec<4, T>(v.y);
			Vec<4, T> x = Vec<4, T>(v.x);

			Vec<4, T> result = (z * m[2]) + m[3];
			result = (y * m[1]) + result;
			result = (x * m[0]) + result;

#ifdef IS_MATHS_DIRECTX_MATHS
			DirectX::XMVECTOR dxVector = DirectX::XMVector3Transform(v.xmvector, m.xmmatrix);
			assert(dxVector.m128_f32[0] == result[0]);
			assert(dxVector.m128_f32[1] == result[1]);
			assert(dxVector.m128_f32[2] == result[2]);
			assert(dxVector.m128_f32[3] == result[3]);
#endif
			return result;
		}
	}
}