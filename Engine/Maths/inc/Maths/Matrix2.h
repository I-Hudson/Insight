#pragma once

#include "Maths/Defines.h"
#include "Vector2.h"
#include "Vector4.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif

namespace Insight
{
	namespace Maths
	{

		/// @brief Two by two matrix (2 by 2). Row majoir.
		class IS_MATHS Matrix2
		{
		public:
			Matrix2();
			Matrix2(float m00, float m01, float m10, float m11);
			Matrix2(const Matrix2& other);
			Matrix2(Matrix2&& other);
#ifdef IS_MATHS_DIRECTX_MATHS
			Matrix2(const DirectX::XMMATRIX& other);
			Matrix2(DirectX::XMMATRIX&& other);
#endif
			~Matrix2();

			/// @brief Inverse this matrix and return a copy (Modifies this matrix).
			/// @return Matrix2
			Matrix2 Inverse();
			/// @brief Return the inverse of this matrix.
			/// @return Matrix2
			Matrix2 Inversed() const { return Matrix2(*this).Inverse(); }
			/// @brief Transpose this matrix and return a copy (Modifies this matrix).
			/// @return Matrix2
			Matrix2 Transpose();
			/// @brief Return the Transpose of this matrix.
			/// @return Matrix2
			Matrix2 Transposed() const { return Matrix2(*this).Transpose(); }

			Vector2& operator[](int i);
			Vector2& operator[](unsigned int i);
			const Vector2& operator[](int i) const;
			const Vector2& operator[](unsigned int i) const;

			bool operator==(const Matrix2& other) const;
			bool operator!=(const Matrix2& other) const;

			Matrix2 operator=(const Matrix2& other);

			Vector2 operator*(const Vector2& other);
			Matrix2 operator*(const Matrix2& other);

			Vector2 operator/(const Vector2& other);
			Matrix2 operator/(const Matrix2& other);

			Matrix2 operator-(const Matrix2& other);

			Matrix2 operator+(const Matrix2& other);

			Matrix2 operator*=(const Vector2& other);
			Matrix2 operator*=(const Matrix2& other);

			Matrix2 operator/=(const Vector2& other);
			Matrix2 operator/=(const Matrix2& other);

			Matrix2 operator-=(const Vector2& other);
			Matrix2 operator-=(const Matrix2& other);

			Matrix2 operator+=(const Vector2& other);
			Matrix2 operator+=(const Matrix2& other);

			const float* Data() const;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				// Vector2 is off size XMVECTOR which is a vector 4.
				struct { DirectX::XMMATRIX xmmatrix; };
				struct 
				{ 
					Vector2 v0; float e02;  float e03;
					Vector2 v1; float e12;  float e13;
					Vector2 v2; float e22;  float e23;
					Vector2 v3; float e32;  float e33;
				};
				struct
				{
					float
						m_00, m_01, m_02, m_03,
						m_10, m_11, m_12, m_13,
						m_20, m_21, m_22, m_23,
						m_30, m_31, m_32, m_33;
				};
#else
				struct { Vector2 data[2]; };
				struct
				{
					float
						m_00, m_01,
						m_10, m_11;
				};
#endif
			};

			static const Matrix2 Zero;
			static const Matrix2 One;
			static const Matrix2 Identity;
		};
	}
}
