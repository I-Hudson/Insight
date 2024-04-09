#pragma once

#include "Maths/Defines.h"
#include "Vector3.h"
#include "Vector4.h"

#ifdef IS_MATHS_DIRECTX_MATHS
#include <DirectXMath.h>
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
#include <glm/glm.hpp>
#endif 

namespace Insight
{
	namespace Maths
	{

		/// @brief three by three matrix (3 by 3). Row majoir.
		class IS_MATHS Matrix3
		{
		public:
			Matrix3();
			Matrix3(float m00, float m01, float m02,
				float m10, float m11, float m12,
				float m20, float m21, float m22);
			Matrix3(Vector3 v0, Vector3 v1, Vector3 v2);
			Matrix3(const Matrix3& other);
			Matrix3(Matrix3&& other);
#ifdef IS_MATHS_DIRECTX_MATHS
			Matrix3(const DirectX::XMMATRIX& other);
			Matrix3(DirectX::XMMATRIX&& other);
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
			Matrix3(const glm::mat3& other);
			Matrix3(glm::mat3&& other);
#endif
			~Matrix3();

			/// @brief Inverse this matrix and return a copy (Modifies this matrix).
			/// @return Matrix3
			Matrix3 Inverse();
			/// @brief Return the inverse of this matrix.
			/// @return Matrix3
			Matrix3 Inversed() const { return Matrix3(*this).Inverse(); }
			/// @brief Transpose this matrix and return a copy (Modifies this matrix).
			/// @return Matrix3
			Matrix3 Transpose();
			/// @brief Return the Transpose of this matrix.
			/// @return Matrix3
			Matrix3 Transposed() const { return Matrix3(*this).Transpose(); }

			Vector3& operator[](int i);
			const Vector3& operator[](int i) const;

			bool operator==(const Matrix3& other) const;
			bool operator!=(const Matrix3& other) const;

			bool Equal(const Matrix3& other, const float errorRange) const;
			bool NotEqual(const Matrix3& other, const float errorRange) const;

			Matrix3 operator=(const Matrix3& other);

			Vector3 operator*(const Vector3& other) const;
			Matrix3 operator*(const Matrix3& other) const;

			Vector3 operator/(const Vector3& other) const;
			Matrix3 operator/(const Matrix3& other) const;

			Matrix3 operator-(const Matrix3& other) const;

			Matrix3 operator+(const Matrix3& other) const;

			Matrix3 operator*=(const Vector3& other);
			Matrix3 operator*=(const Matrix3& other);

			Matrix3 operator/=(const Vector3& other);
			Matrix3 operator/=(const Matrix3& other);

			Matrix3 operator-=(const Vector3& other);
			Matrix3 operator-=(const Matrix3& other);

			Matrix3 operator+=(const Vector3& other);
			Matrix3 operator+=(const Matrix3& other);

			const float* Data() const;

			union
			{
#ifdef IS_MATHS_DIRECTX_MATHS
				// Vector3 is off size XMVECTOR which is a vector 4.
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
						m_00, m_01, m_02,
						m_10, m_11, m_12,
						m_20, m_21, m_22;
				};
#endif
#if defined(IS_MATHS_GLM) || defined(IS_TESTING)
				struct { glm::mat3 mat3; };
#endif
				struct
				{
					Vector3 v0;
					Vector3 v1;
					Vector3 v2;
				};

			};

			static const Matrix3 Zero;
			static const Matrix3 One;
			static const Matrix3 Identity;
		};
	}
}