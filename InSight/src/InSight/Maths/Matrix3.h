#pragma once

#include "Insight/Core.h"
#include "Insight/Maths/Vector3.h"

namespace Insight
{
	namespace Maths
	{
		class IS_API Matrix3
		{
		public:
			//\===========================================================================================
			//\ Constants
			//\===========================================================================================
			static const Matrix3 ONE;
			static const Matrix3 ZERO;
			static const Matrix3 IDENTITY;
			//\===========================================================================================
			//\ Constructors
			//\===========================================================================================
			Matrix3();
			Matrix3(const float* mat);
			Matrix3(float m11, float m12, float m13,
				float m21, float m22, float m23,
				float m31, float m32, float m33);
			Matrix3(Matrix3& a_m3);
			//\===========================================================================================
			//\ Casting operators
			//\===========================================================================================
			operator float* ();
			operator const float* () const;
			//\===========================================================================================
			//\ Component access operators
			//\===========================================================================================
			float&					operator			() (int iRow, int iCol);
			float					operator			() (int iRow, int iCol) const;
			//\===========================================================================================
			//\ Cols & Rows Access
			//\===========================================================================================
			void					SetCol(int iCol, const Vector3& vCol);
			void					SetRow(int iRow, const Vector3& vRow);
			Vector3					GetCol(int iCol) const;
			Vector3					GetRow(int iRow) const;
			void					GetCol(int iCol, Vector3& vCol) const;
			void					GetRow(int iRow, Vector3& vRow) const;
			//\===========================================================================================
			//\ Equivalence Operators
			//\===========================================================================================
			bool					operator  ==		(const Matrix3& a_m3) const;
			bool					operator  !=		(const Matrix3& a_m3) const;

			//\===========================================================================================
			//\ Operator overloads for Addition
			//\===========================================================================================
			const Matrix3			operator  +			(const Matrix3& a_m3) const;
			const Matrix3&			operator  +=		(const Matrix3& a_m3);
			//\===========================================================================================
			//\ Operator overloads for Subtraction
			//\===========================================================================================  
			const Matrix3           operator-			(const Matrix3& a_m3) const;
			const Matrix3&			operator  -=		(const Matrix3& a_m3);
			//\===========================================================================================
			//\ Operator overloads for multiplication
			//\===========================================================================================
			const Matrix3			operator  *			(const float a_fScalar) const;
			friend  const Matrix3	operator  *			(const float a_fScalar, const Matrix3& a_m3);
			const Matrix3&			operator  *=		(const float a_fScalar);

			const Vector3			operator  *			(const Vector3& a_v3) const;
			friend  const Matrix3 	operator  *			(const Vector3& a_v3, const Matrix3& a_m3);

			const Matrix3			operator  *			(const Matrix3& a_m3) const;
			const Matrix3&			operator  *=		(const Matrix3& a_m3);
			//\===========================================================================================
			//\ Transpose Matrix - Transform from Row to Column
			//\===========================================================================================
			void   					Transpose();
			void   					GetTranspose(Matrix3& a_m3);
			//\===========================================================================================
			//\ Inverse - Get the inverse of the matrix
			//\===========================================================================================
			float  					Determinant() const;
			bool   					Inverse();
			bool   					GetInverse(Matrix3& a_m3)const;
			//\===========================================================================================
			//\ Rotation Functions
			//\===========================================================================================
			void   					 RotateX(float fAngle);
			void   					 RotateY(float fAngle);
			void   					 RotateZ(float fAngle);
			static Matrix3			 M3CreateRotateX(float fAngle);
			static Matrix3			 M3CreateRotateY(float fAngle);
			static Matrix3			 M3CreateRotateZ(float fAngle);
			//\===========================================================================================
			//\ Scale Functionality
			//\===========================================================================================
			void    				 Scale(const Vector3& a_v3);
			void   					 Scale(float a_fScalar);
			//\===========================================================================================
			//\ General matrix functions
			//\===========================================================================================
			void   					 Identity();
			void   					 Zero();

			//\===========================================================================================
			//\ Member Variables held in unnamed union for accessibility
			//\===========================================================================================
			union
			{
				struct
				{
					float	m[3][3];
				};
				struct
				{
					float f[9];
				};
				struct
				{
					// m_11, m_12, m_13 = x
					// m_21, m_22, m_23 = y
					// m_31, m_32, m_33 = z
					float	m_11, m_12, m_13;
					float	m_21, m_22, m_23;
					float	m_31, m_32, m_33;
				};
			};

		};
	}
}