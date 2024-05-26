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

		/// @brief Four by Four matrix (4 by 4). Row majoir.
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
			Matrix4 Inverse();
			/// @brief Return the inverse of this matrix.
			/// @return Matrix4
			Matrix4 Inversed() const { return Matrix4(*this).Inverse(); }
			/// @brief Transpose this matrix and return a copy (Modifies this matrix).
			/// @return Matrix4
			Matrix4 Transpose();
			/// @brief Return the Transpose of this matrix.
			/// @return Matrix4
			Matrix4 Transposed() const { return Matrix4(*this).Transpose(); }

			Matrix4 Translate(const Vector4 vector);
			Matrix4 Translated(const Vector4 vector) const { return Matrix4(*this).Translate(vector); }

			Matrix4 Scale(const Vector4& vector);
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
	}
}