#pragma once

#include "Maths/Defines.h"
#include "Vector2.h"

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
				struct { Vector2 data[2]; };
				struct
				{
					float
						m_00, m_01,
						m_10, m_11;
				};
			};

			static const Matrix2 Zero;
			static const Matrix2 One;
			static const Matrix2 Identity;
		};
	}
}
