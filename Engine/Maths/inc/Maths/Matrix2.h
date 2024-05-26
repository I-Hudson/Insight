#pragma once

#include "Maths/Defines.h"
#include "Vector2.h"
#include "Vector4.h"

#include <Reflect/Core/Defines.h>

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{

		/// @brief Two by two matrix (2 by 2). Row majoir.
		REFLECT_CLASS(REFLECT_LOOKUP_ONLY);
		class IS_MATHS Matrix2
		{
		public:
			Matrix2();
			Matrix2(float m00, float m01, float m10, float m11);
			Matrix2(const Matrix2& other);
			Matrix2(Matrix2&& other);
			Matrix2(const Vector2& v1, const Vector2& v2);
#ifdef IS_MATHS_DIRECTX_MATHS
			Matrix2(const DirectX::XMMATRIX& other);
			Matrix2(DirectX::XMMATRIX&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
			Matrix2(const glm::mat2& other);
			Matrix2(glm::mat2&& other);
#endif
			~Matrix2();

			/// @brief Inverse this matrix and return a copy (Modifies this matrix).
			/// @return Matrix2
			Matrix2 Inverse();
			/// @brief Return the inverse of this matrix.
			/// @return Matrix2
			Matrix2 Inversed() const;
			/// @brief Transpose this matrix and return a copy (Modifies this matrix).
			/// @return Matrix2
			Matrix2 Transpose();
			/// @brief Return the Transpose of this matrix.
			/// @return Matrix2
			Matrix2 Transposed() const;

			Vector2& operator[](int i);
			Vector2& operator[](unsigned int i);
			const Vector2& operator[](int i) const;
			const Vector2& operator[](unsigned int i) const;

			bool operator==(const Matrix2& other) const;
			bool operator!=(const Matrix2& other) const;

			bool Equal(const Matrix2& other, const float errorRange) const;
			bool NotEqual(const Matrix2& other, const float errorRange) const;

			Matrix2& operator=(const Matrix2& other);

			Vector2 operator*(const Vector2& other);
			Matrix2 operator*(const Matrix2& other);

			Vector2 operator/(const Vector2& other);
			Matrix2 operator/(const Matrix2& other);

			Matrix2 operator-(const Matrix2& other);

			Matrix2 operator+(const Matrix2& other);

			Matrix2& operator*=(const Vector2& other);
			Matrix2& operator*=(const Matrix2& other);

			Matrix2& operator/=(const Vector2& other);
			Matrix2& operator/=(const Matrix2& other);

			Matrix2& operator-=(const Vector2& other);
			Matrix2& operator-=(const Matrix2& other);

			Matrix2& operator+=(const Vector2& other);
			Matrix2& operator+=(const Matrix2& other);

			const float* Data() const;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				// Vector2 is off size XMVECTOR which is a vector 4.
				struct { DirectX::XMMATRIX xmmatrix; };
				struct
				{
					float
						m_00, m_01, m_02, m_03,
						m_10, m_11, m_12, m_13,
						m_20, m_21, m_22, m_23,
						m_30, m_31, m_32, m_33;
				};
#else
				struct
				{
					float
						m_00, m_01,
						m_10, m_11;
				};
#endif
#if defined(IS_MATHS_GLM) || defined(IS_MATHS_CONSTRUCTOR_GLM) || defined(IS_TESTING)
				struct { glm::mat2 mat2; };
#endif
				struct
				{
					Vector2 v0;
					Vector2 v1;
				};
			};

			static const Matrix2 Zero;
			static const Matrix2 One;
			static const Matrix2 Identity;
		};
	}
}
