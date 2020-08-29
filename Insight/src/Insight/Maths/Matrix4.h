#pragma once 

#include "Insight/Core.h"
#include "Insight/Maths/Matrix3.h"
#include "Insight/Maths/Vector2.h"
#include "Insight/Maths/Vector4.h"

namespace Insight
{
	namespace Maths
	{
		class IS_API Matrix4
		{
		public:
			//\===========================================================================================
			//\ Constants
			//\===========================================================================================
			static const Matrix4 ZERO;
			static const Matrix4 ONE;
			static const Matrix4 IDENTITY;
			//\===========================================================================================
			//\ Constructors
			//\===========================================================================================
			Matrix4();
			explicit Matrix4(const float* a_mat);
			Matrix4(float m11, float m12, float m13, float m14,
				float m21, float m22, float m23, float m24,
				float m31, float m32, float m33, float m34,
				float m41, float m42, float m43, float m44);
			Matrix4(const Matrix3& a_m3);
			Matrix4(const Matrix4& a_m4);
			//\===========================================================================================
			//\ Casting operators
			//\===========================================================================================
			operator float* ();
			operator const float* () const;
			//\===========================================================================================
			//\ Component Access Operators
			//\===========================================================================================
			float& operator			() (int iRow, int iCol);
			float					operator			() (int iRow, int iCol) const;
			//\===========================================================================================
			//\ Component Access/ Retreival As Vector4
			//\===========================================================================================
			void					SetX(const Vector4& a_v4);
			void					SetY(const Vector4& a_v4);
			void					SetZ(const Vector4& a_v4);
			void					SetT(const Vector4& a_v4);
			Vector4					GetX() const;
			Vector4					GetY() const;
			Vector4					GetZ() const;
			Vector4					GetT() const;
			//\===========================================================================================
			//\ Component Access/ Retreival As Vector3
			//\===========================================================================================
			void					SetX(const Vector3& a_v3);
			void					SetY(const Vector3& a_v3);
			void					SetZ(const Vector3& a_v3);
			void					SetT(const Vector3& a_v3);
			void					GetX(Vector3& a_v3) const;
			void					GetY(Vector3& a_v3) const;
			void					GetZ(Vector3& a_v3) const;
			void					GetT(Vector3& a_v3) const;
			//\===========================================================================================
			//\ Component Row/Column Access Operators
			//\===========================================================================================
			void					SetCol(int iCol, const Vector3& vCol);
			void					SetCol(int iCol, const Vector4& vCol);

			void					GetCol(int iCol, Vector3& vCol) const;
			void					GetCol(int iCol, Vector4& vCol) const;

			void					SetRow(int iRow, const Vector3& vRow);
			void					SetRow(int iRow, const Vector4& vRow);

			void					GetRow(int iRow, Vector3& vRow) const;
			void					GetRow(int iRow, Vector4& vRow) const;
			//\===========================================================================================
			//\ Equivalence operators
			//\===========================================================================================
			bool					operator ==			(const Matrix4& a_m4) const;
			bool					operator !=			(const Matrix4& a_m4) const;
			//\===========================================================================================
			//\ Operator overloads for Addition
			//\===========================================================================================
			const Matrix4			operator  +			(const Matrix4& a_m4) const;
			const Matrix4& operator  +=		(const Matrix4& a_m4);
			//\===========================================================================================
			//\ Operator overloads for Subtraction
			//\===========================================================================================
			const Matrix4			operator  -			(const Matrix4& a_m4) const;
			const Matrix4& operator  -=		(const Matrix4& a_m4);
			//\===========================================================================================
			//\ Operator overloads for multiplication
			//\===========================================================================================	
			const Matrix4			operator *			(float a_fScalar) const;
			const Matrix4& operator *=			(float a_fScalar);

			const Vector3			operator *			(const Vector3& a_v3) const;

			const Vector4			operator *			(const Vector4& a_v4) const;

			const Matrix4			operator *			(const Matrix4& a_m4) const;
			const Matrix4& operator *=			(const Matrix4& a_m4);
			//\===========================================================================================	
			//\ Transpose Matrix
			//\===========================================================================================
			void				Transpose();
			void				GetTranspose(Matrix4& mat);
			//\===========================================================================================	
			// General matrix functions
			//\===========================================================================================
			void				Scale(const Vector3& a_v3);
			void				Scale(float a_fScalar);
			//\===========================================================================================	
			// Inverse Matrix Functions
			//\===========================================================================================
			float 				Determinant() const;
			float 				Determinant3() const;
			bool				Inverse();
			bool				GetInverse(Matrix4& mat) const;
			//\===========================================================================================	
			// Matrix Rotation functions
			//\===========================================================================================
			void  				RotateX(float fAngle);
			void  				RotateY(float fAngle);
			void  				RotateZ(float fAngle);

			friend Matrix4  	CreateRotateX(float fAngle);
			friend Matrix4	    CreateRotateY(float fAngle);
			friend Matrix4		CreateRotateZ(float fAngle);
			//\===========================================================================================	
			// General matrix functions
			//\===========================================================================================
			void				Identity();
			void				Zero();

			void Translate(const Vector2 a_v2);
			void Translate(const Vector3 a_v3);

			void MoveForwards(const float a_f);
			void MoveBackwards(const float a_f);

			//\===========================================================================================
			//\ Member Variables held in unnamed union for accessibility
			//\===========================================================================================
			union
			{
				struct
				{
					float			m[4][4];
				};
				struct
				{
					float           f[16];
				};
				struct
				{
					float		m_11, m_12, m_13, m_14;
					float		m_21, m_22, m_23, m_24;
					float		m_31, m_32, m_33, m_34;
					float		m_41, m_42, m_43, m_44;
				};
			};
		};
	}
}