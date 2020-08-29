#include "ispch.h"
#include "Insight/Maths/Matrix3.h"


namespace Insight
{
	namespace Maths
	{
		//When IDENITIY is called on a Matrix initialized to this
		const Matrix3 Matrix3::IDENTITY = Matrix3(1.f, 0.f, 0.f,
			0.f, 1.f, 0.f,
			0.f, 0.f, 1.f);


		Matrix3::Matrix3() : m_11(0), m_12(0), m_13(0),
			m_21(0), m_22(0), m_23(0),
			m_31(0), m_32(0), m_33(0)
		{
		}

		Matrix3::Matrix3(const float* a_mat) : m_11(a_mat[0]), m_12(a_mat[1]), m_13(a_mat[2]),
			m_21(a_mat[3]), m_22(a_mat[4]), m_23(a_mat[5]),
			m_31(a_mat[6]), m_32(a_mat[7]), m_33(a_mat[8])
		{
		}

		Matrix3::Matrix3(float a_m_11, float a_m_12, float a_m_13, float a_m_21, float a_m_22, float a_m_23, float a_m_31, float a_m_32, float a_m_33) :
			m_11(a_m_11), m_12(a_m_12), m_13(a_m_13),
			m_21(a_m_21), m_22(a_m_22), m_23(a_m_23),
			m_31(a_m_31), m_32(a_m_32), m_33(a_m_33)
		{
		}

		Matrix3::Matrix3(Matrix3& a_m3) :
			m_11(a_m3.m_11), m_12(a_m3.m_12), m_13(a_m3.m_13),
			m_21(a_m3.m_21), m_22(a_m3.m_22), m_23(a_m3.m_23),
			m_31(a_m3.m_31), m_32(a_m3.m_32), m_33(a_m3.m_33)
		{
		}

		//Used for compatible: GL When reading in the first float all the floats net to it in the array will be place in memory next the each
		//other, this will allow the first float to be cast then grab the memory next to it for the rest
		Matrix3::operator float* ()
		{
			return static_cast<float*>(&m_11);
		}
		//Used for compatible
		Matrix3::operator const float* () const
		{
			return static_cast<const float*>(&m_11);
		}

		//return value at specified row and col
		float& Matrix3::operator()(int iRow, int iCol)
		{
			return m[iRow][iCol];
		}

		//return value at specified row and col
		float Matrix3::operator()(int iRow, int iCol) const
		{
			return m[iRow][iCol];
		}

		//used to select the col in the Matrix which will be set, pass though a vector 3 for the value in the col selected
		void Matrix3::SetCol(int iCol, const Vector3& vCol)
		{
			m[0][iCol] = vCol.x;
			m[1][iCol] = vCol.y;
			m[2][iCol] = vCol.z;
		}

		//used to select the row in the Matrix which will be set, pass though a vector 3 for the value in the row selected 
		void Matrix3::SetRow(int iRow, const Vector3& vRow)
		{
			m[iRow][0] = vRow.x;
			m[iRow][1] = vRow.y;
			m[iRow][2] = vRow.z;
		}

		//used the get the col selected, returned in a vector3
		Vector3 Matrix3::GetCol(int iCol) const
		{
			return Vector3(m[0][iCol], m[1][iCol], m[2][iCol]);
		}

		//used the get the row selected, returned in a vector3, 
		Vector3 Matrix3::GetRow(int iRow) const
		{
			return Vector3(m[iRow][0], m[iRow][1], m[iRow][2]);
		}

		//used to get col selected, col value selected are passed in to a vector3
		void Matrix3::GetCol(int iCol, Vector3& vCol) const
		{
			vCol.Set(m[0][iCol], m[1][iCol], m[2][iCol]);
		}

		//used to get row selected, row value selected are passed in to a vector3
		void Matrix3::GetRow(int iRow, Vector3& vRow) const
		{
			vRow.Set(m[iRow][0], m[iRow][1], m[iRow][2]);
		}

		//if the Matrix3 is equal to Matrix3 a_m3 return true
		bool Matrix3::operator==(const Matrix3& a_m3) const
		{
			return ((m_11 == a_m3.m_11) && (m_12 == a_m3.m_12) && (m_13 == a_m3.m_13) &&
				(m_21 == a_m3.m_21) && (m_22 == a_m3.m_22) && (m_23 == a_m3.m_23) &&
				(m_31 == a_m3.m_31) && (m_32 == a_m3.m_32) && (m_33 == a_m3.m_33));

		}

		//if the Matrix3 is not equal to Matrix a_m3 return true 
		bool Matrix3::operator!=(const Matrix3& a_m3) const
		{
			return ((m_11 != a_m3.m_11) || (m_12 != a_m3.m_12) || (m_13 != a_m3.m_13) ||
				(m_21 != a_m3.m_21) || (m_22 != a_m3.m_22) || (m_23 != a_m3.m_23) ||
				(m_31 != a_m3.m_31) || (m_32 != a_m3.m_32) || (m_33 != a_m3.m_33));


		}

		//used to add Matrix3 to Matrix3 a_m3, return a Martix 3
		const Matrix3 Matrix3::operator+(const Matrix3& a_m3) const
		{
			return Matrix3(m_11 + a_m3.m_11, m_12 + a_m3.m_12, m_13 + a_m3.m_13,
				m_21 + a_m3.m_21, m_22 + a_m3.m_22, m_23 + a_m3.m_23,
				m_31 + a_m3.m_31, m_32 + a_m3.m_32, m_33 + a_m3.m_33);
		}

		//used to add Matrix 3 to Matrix 3 a_m3 
		const Matrix3& Matrix3::operator+=(const Matrix3& a_m3)
		{
			m_11 += a_m3.m_11;
			m_12 += a_m3.m_12;
			m_13 += a_m3.m_13;
			m_21 += a_m3.m_21;
			m_22 += a_m3.m_22;
			m_23 += a_m3.m_23;
			m_31 += a_m3.m_31;
			m_32 += a_m3.m_32;
			m_33 += a_m3.m_33;
			return *this;
		}

		//used to subtract Matrix3 by Matrixa_m3, returns a Martix 3
		const Matrix3 Matrix3::operator-(const Matrix3& a_m3) const
		{
			return Matrix3
			(m_11 - a_m3.m_11, m_12 - a_m3.m_12, m_13 - a_m3.m_13,
				m_21 - a_m3.m_21, m_22 - a_m3.m_22, m_23 - a_m3.m_23,
				m_31 - a_m3.m_31, m_32 - a_m3.m_32, m_33 - a_m3.m_33);
		}

		//used to subtract Matrix 3 by Matrix 3 a_m3 elements
		const Matrix3& Matrix3::operator-=(const Matrix3& a_m3)
		{
			m_11 -= a_m3.m_11;
			m_12 -= a_m3.m_12;
			m_13 -= a_m3.m_13;
			m_21 -= a_m3.m_21;
			m_22 -= a_m3.m_22;
			m_23 -= a_m3.m_23;
			m_31 -= a_m3.m_31;
			m_32 -= a_m3.m_32;
			m_33 -= a_m3.m_33;
			return *this;
		}

		//used the multiply Matrix3 by a a_fScalar, returns a Matrix 3
		const Matrix3 Matrix3::operator*(const float a_fScalar) const
		{
			return Matrix3(m_11 * a_fScalar, m_12 * a_fScalar, m_13 * a_fScalar,
				m_21 * a_fScalar, m_22 * a_fScalar, m_23 * a_fScalar,
				m_31 * a_fScalar, m_32 * a_fScalar, m_33 * a_fScalar);
		}


		const  Matrix3 operator*(const float a_fScalar, const Matrix3& a_m3)
		{
			return(a_m3 * a_fScalar);
		}

		//use the multiply Matrix 3 by a_fScaler 
		const Matrix3& Matrix3::operator*=(const float a_fScalar)
		{
			m_11 *= a_fScalar;		m_12 *= a_fScalar;		m_13 *= a_fScalar;
			m_21 *= a_fScalar;		m_22 *= a_fScalar;		m_23 *= a_fScalar;
			m_31 *= a_fScalar;		m_32 *= a_fScalar;		m_33 *= a_fScalar;
			return *this;
		}

		//used to multiply a Matrix3 by a Vector3, then return each axis in the Matrix3 in a vector3.
		//E.G: X axis (row 0) is returned in the X position in the vector
		const Vector3 Matrix3::operator*(const Vector3& a_v3) const
		{
			return Vector3(a_v3.x * m[0][0] + a_v3.y * m[0][1] + a_v3.z * m[0][2],
				a_v3.x * m[1][0] + a_v3.y * m[1][1] + a_v3.z * m[1][2],
				a_v3.x * m[2][0] + a_v3.y * m[2][1] + a_v3.z * m[2][2]);
		}

		const Matrix3 operator*(const Vector3& a_v3, const Matrix3& a_m3)
		{
			//return(a_v3 * a_m3);
			a_v3;
			a_m3;
			return 0;
		}


		//used to multiply Matrix3 by Matrix3 a_m3, returns a Matrix 3 
		const Matrix3 Matrix3::operator*(const Matrix3& a_m3) const
		{
			return Matrix3(m_11 * a_m3.m_11 + m_12 * a_m3.m_21 + m_13 * a_m3.m_31,
				m_11 * a_m3.m_12 + m_12 * a_m3.m_22 + m_13 * a_m3.m_32,
				m_11 * a_m3.m_13 + m_12 * a_m3.m_23 + m_13 * a_m3.m_33,

				m_21 * a_m3.m_11 + m_22 * a_m3.m_21 + m_23 * a_m3.m_31,
				m_21 * a_m3.m_12 + m_22 * a_m3.m_22 + m_23 * a_m3.m_32,
				m_21 * a_m3.m_13 + m_22 * a_m3.m_23 + m_23 * a_m3.m_33,

				m_31 * a_m3.m_11 + m_32 * a_m3.m_21 + m_33 * a_m3.m_31,
				m_31 * a_m3.m_12 + m_32 * a_m3.m_22 + m_33 * a_m3.m_32,
				m_31 * a_m3.m_13 + m_32 * a_m3.m_32 + m_33 * a_m3.m_33);
		}

		//used to multiply 3 elements by Matrix 3 a_m3
		const Matrix3& Matrix3::operator*=(const Matrix3& a_m3)
		{
			m_11 *= a_m3.m_11;
			m_12 *= a_m3.m_12;
			m_13 *= a_m3.m_13;
			m_21 *= a_m3.m_21;
			m_22 *= a_m3.m_22;
			m_23 *= a_m3.m_23;
			m_31 *= a_m3.m_31;
			m_32 *= a_m3.m_32;
			m_33 *= a_m3.m_33;
			return *this;


		}

		//used to transpose Matrix 3
		void  Matrix3::Transpose()
		{
			float k = m_12;
			m_12 = m_21;
			m_21 = k;

			k = m_31;
			m_13 = m_31;
			m_31 = k;

			k = m_23;
			m_32 = m_23;
			m_23 = k;


		}

		//used to pass thought the transpose values into Matrix a_m3
		void Matrix3::GetTranspose(Matrix3& a_m3)
		{
			Transpose();

			a_m3.m_11 = m_11;	a_m3.m_12 = m_12;	a_m3.m_13 = m_13;
			a_m3.m_21 = m_21;	a_m3.m_22 = m_22;	a_m3.m_23 = m_23;
			a_m3.m_31 = m_31;	a_m3.m_32 = m_32;	a_m3.m_33 = m_33;

		}

		//
		float Matrix3::Determinant() const
		{
			return	(m_11 * (m_22 * m_33 - m_23 * m_32) +
				m_12 * (m_23 * m_31 - m_21 * m_33) +
				m_13 * (m_21 * m_32 - m_22 * m_31));
		}

		//Inverse - Get the inverse of the matrix
		bool Matrix3::Inverse()
		{
			float fDeterminant = Determinant();
			if (fDeterminant != 0.0f)
			{
				const float fInvDet = 1 / fDeterminant;

				Matrix3 mat = *this;
				m_11 = (mat.m_22 * mat.m_33 - mat.m_23 * mat.m_32) * fInvDet;
				m_12 = (mat.m_13 * mat.m_32 - mat.m_12 * mat.m_33) * fInvDet;
				m_13 = (mat.m_12 * mat.m_23 - mat.m_13 * mat.m_22) * fInvDet;

				m_21 = (mat.m_23 * mat.m_31 - mat.m_21 * mat.m_33) * fInvDet;
				m_22 = (mat.m_11 * mat.m_33 - mat.m_13 * mat.m_31) * fInvDet;
				m_23 = (mat.m_13 * mat.m_21 - mat.m_11 * mat.m_23) * fInvDet;

				m_31 = (mat.m_21 * mat.m_32 - mat.m_22 * mat.m_31) * fInvDet;
				m_32 = (mat.m_12 * mat.m_31 - mat.m_11 * mat.m_32) * fInvDet;
				m_33 = (mat.m_11 * mat.m_22 - mat.m_12 * mat.m_21) * fInvDet;

				return true;
			}
			return false;
		}

		//
		bool Matrix3::GetInverse(Matrix3& a_m3) const
		{
			Matrix3 mm = a_m3;

			return false;
		}

		//used to rotate Matrix 3 on the axis
		//this will allow this function to be called and for t he rotation to be applied to the Matrix3 with no other input
		void Matrix3::RotateX(float fAngle)
		{
			*this = M3CreateRotateX(fAngle) * *this;
		}

		//used to rotate Matrix 3 on the Y axis
		//this will allow this function to be called and for t he rotation to be applied to the Matrix3 with no other input
		void Matrix3::RotateY(float fAngle)
		{
			*this = M3CreateRotateY(fAngle) * *this;
		}

		// used to rotate Matrix 3 on the Z axis
		//this will allow this function to be called and for t he rotation to be applied to the Matrix3 with no other input
		void Matrix3::RotateZ(float fAngle)
		{
			*this = M3CreateRotateZ(fAngle) * *this;
		}

		//Creates a Matrix3, then rotates it on the X axis
		Matrix3 Matrix3::M3CreateRotateX(float fAngle)
		{
			Matrix3 m3;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m3.m_11 = 1.0f, m3.m_12 = 0.f, m3.m_13 = 0.f;
			m3.m_21 = 0.f, m3.m_22 = c, m3.m_23 = s;
			m3.m_31 = 0.f, m3.m_32 = -s, m3.m_33 = c;

			return m3;
		}

		//Creates a Matrix3, then rotates it on the Y axis
		Matrix3 Matrix3::M3CreateRotateY(float fAngle)
		{
			Matrix3 m3;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m3.m_11 = c, m3.m_12 = 0.f, m3.m_13 = -s;
			m3.m_21 = 0.f, m3.m_22 = 1.f, m3.m_23 = 0.f;
			m3.m_31 = s, m3.m_32 = 0.f, m3.m_33 = c;

			return m3;
		}

		//Creates a Matrix3, then rotates it on the XZaxis
		Matrix3 Matrix3::M3CreateRotateZ(float fAngle)
		{
			Matrix3 m3;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m3.m_11 = c, m3.m_12 = s, m3.m_13 = 0.f;
			m3.m_21 = -s, m3.m_22 = c, m3.m_23 = 0.f;
			m3.m_31 = 0.f, m3.m_32 = 0.f, m3.m_33 = 1.f;

			return m3;
		}
		//used to scale Matrix 3 by Vector 3 a_v3
		void Matrix3::Scale(const Vector3& a_v3)
		{
			m_11* a_v3.x,
				m_12* a_v3.x,
				m_13* a_v3.x,

				m_21* a_v3.y,
				m_22* a_v3.y,
				m_23* a_v3.y,

				m_31* a_v3.z,
				m_32* a_v3.z,
				m_33* a_v3.z;
		}

		//used to scale Matrix 3 by a_fScalar
		void Matrix3::Scale(float a_fScalar)
		{
			m_11 = a_fScalar, m_12 = 0.f, m_13 = 0.f,
				m_21 = 0.f, m_22 = a_fScalar, m_23 = 0.f,
				m_31 = 0.f, m_32 = 0.f, m_33 = a_fScalar;
		}

		//used to make Matrix 3 in to an Identity Matrix 
		void Matrix3::Identity()
		{
			Matrix3(1.f, 0.f, 0.f,
				0.f, 1.f, 0.f,
				0.f, 0.f, 1.f);
		}
		void Matrix3::Zero()
		{
			Matrix3(0.f, 0.f, 0.f,
				0.f, 0.f, 0.f,
				0.f, 0.f, 0.f);
		}
	}
}