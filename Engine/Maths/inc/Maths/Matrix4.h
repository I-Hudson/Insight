#pragma once

#include "Maths/Defines.h"
#include "Vector4.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif

namespace Insight
{
	namespace Maths
	{

		/// @brief Four by Four matrix (4 by 4). Row majoir.
		class IS_MATHS Matrix4
		{
		public:
			Matrix4();
			Matrix4(float m00, float m01, float m02, float m_03,
				float m10, float m11, float m12, float m_13,
				float m20, float m21, float m22, float m_23,
				float m30, float m31, float m32, float m_33);
			Matrix4(const Matrix4& other);
			Matrix4(Matrix4&& other);
#ifdef IS_MATHS_DIRECTX_MATHS
			Matrix4(const DirectX::XMMATRIX& other);
			Matrix4(DirectX::XMMATRIX&& other);
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

			Vector4& operator[](int i);
			Vector4& operator[](unsigned int i);
			const Vector4& operator[](int i) const;
			const Vector4& operator[](unsigned int i) const;

			bool operator==(const Matrix4& other) const;
			bool operator!=(const Matrix4& other) const;

			Matrix4 operator=(const Matrix4& other);

			Matrix4 operator*(const Matrix4& other);
			Vector4 operator*(const Vector4& other);

			Matrix4 operator/(const Matrix4& other);
			Vector4 operator/(const Vector4& other);

			Matrix4 operator-(const Matrix4& other);

			Matrix4 operator+(const Matrix4& other);

			Matrix4 operator*=(const Matrix4& other);
			/// @brief Multiple each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4 operator*=(const Vector4& other);

			Matrix4 operator/=(const Matrix4& other);
			/// @brief Divide each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4 operator/=(const Vector4& other);

			Matrix4 operator-=(const Matrix4& other);
			/// @brief Subtract each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4 operator-=(const Vector4& other);

			Matrix4 operator+=(const Matrix4& other);
			/// @brief Add each row of the matrix by the vector as a scaler.
			/// @param other 
			/// @return Matrix4
			Matrix4 operator+=(const Vector4& other);

			const float* Data() const;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				// Vector4 is off size XMVECTOR which is a vector 4.
				struct { DirectX::XMMATRIX xmmatrix; };
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
	}
}