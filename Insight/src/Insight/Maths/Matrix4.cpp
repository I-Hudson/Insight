#include "ispch.h"
#include "Insight/Maths/Matrix4.h"

namespace Insight
{
	namespace Maths
	{

		const Matrix4 Matrix4::ZERO = Matrix4(0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f);

		const Matrix4 Matrix4::ONE = Matrix4(1.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 1.f, 1.f, 1.f);

		const Matrix4 Matrix4::IDENTITY = Matrix4(1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f);

		Matrix4::Matrix4() :m_11(0), m_12(0), m_13(0), m_14(0),
			m_21(0), m_22(0), m_23(0), m_24(0),
			m_31(0), m_32(0), m_33(0), m_34(0),
			m_41(0), m_42(0), m_43(0), m_44(1)
		{
		}

		Matrix4::Matrix4(const float* a_mat) :m_11(a_mat[0]), m_12(a_mat[1]), m_13(a_mat[2]), m_14(a_mat[3]),
			m_21(a_mat[4]), m_22(a_mat[5]), m_23(a_mat[6]), m_24(a_mat[7]),
			m_31(a_mat[8]), m_32(a_mat[9]), m_33(a_mat[10]), m_34(a_mat[11]),
			m_41(a_mat[12]), m_42(a_mat[13]), m_43(a_mat[14]), m_44(a_mat[15])
		{
		}

		Matrix4::Matrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) :
			m_11(m11), m_12(m12), m_13(m13), m_14(m14),
			m_21(m21), m_22(m22), m_23(m23), m_24(m24),
			m_31(m31), m_32(m32), m_33(m33), m_34(m34),
			m_41(m41), m_42(m42), m_43(m43), m_44(m44)
		{
		}

		Matrix4::Matrix4(const Matrix3& a_m4) :m_11(a_m4.m_11), m_12(a_m4.m_12), m_13(a_m4.m_13), m_14(0),
			m_21(a_m4.m_21), m_22(a_m4.m_22), m_23(a_m4.m_23), m_24(0),
			m_31(a_m4.m_31), m_32(a_m4.m_32), m_33(a_m4.m_33), m_34(0),
			m_41(0), m_42(0), m_43(0), m_44(1)
		{
		}

		Matrix4::Matrix4(const Matrix4& a_m4) :m_11(a_m4.m_11), m_12(a_m4.m_12), m_13(a_m4.m_13), m_14(a_m4.m_14),
			m_21(a_m4.m_21), m_22(a_m4.m_22), m_23(a_m4.m_23), m_24(a_m4.m_24),
			m_31(a_m4.m_31), m_32(a_m4.m_32), m_33(a_m4.m_33), m_34(a_m4.m_13),
			m_41(a_m4.m_41), m_42(a_m4.m_42), m_43(a_m4.m_43), m_44(a_m4.m_44)
		{
		}

		Matrix4::operator float* ()
		{
			return static_cast<float*>(&m_11);
		}

		Matrix4::operator const float* () const
		{
			return static_cast<const float*>(&m_11);
		}

		float& Matrix4::operator()(int iRow, int iCol)
		{
			return m[iRow][iCol];
		}

		//used to set the X axis to what is in Vector 3 a_v4
		void Matrix4::SetX(const Vector4& a_v4)
		{
			m[0][0] = a_v4.x;
			m[0][1] = a_v4.y;
			m[0][2] = a_v4.z;
			m[0][3] = a_v4.w;
		}

		//used to set the Y axis to what is in Vector 4 a_v4
		void Matrix4::SetY(const Vector4& a_v4)
		{
			m[1][0] = a_v4.x;
			m[1][1] = a_v4.y;
			m[1][2] = a_v4.z;
			m[1][3] = a_v4.w;
		}

		//used to set the Z axis to what is in Vector 4 a_v4
		void Matrix4::SetZ(const Vector4& a_v4)
		{
			m[2][0] = a_v4.x;
			m[2][1] = a_v4.y;
			m[2][2] = a_v4.z;
			m[2][3] = a_v4.w;
		}

		//used to set the T axis to what is in Vector 4 a_v4
		void Matrix4::SetT(const Vector4& a_v4)
		{
			m[3][0] = a_v4.x;
			m[3][1] = a_v4.y;
			m[3][2] = a_v4.z;
			m[3][3] = a_v4.w;
		}

		//Retrieve X axis and pass it though Vector 4 a_v4
		Vector4 Matrix4::GetX()const
		{
			return Vector4(m_11, m_12, m_13, m_14);
		}

		//Retrieve Y axis and pass it though Vector 4 a_v4
		Vector4 Matrix4::GetY()const
		{
			return Vector4(m_21, m_22, m_23, m_24);
		}

		//Retrieve Z axis and pass it though Vector 4 a_v4
		Vector4 Matrix4::GetZ()const
		{
			return Vector4(m_31, m_32, m_33, m_34);
		}

		//Retrieve T axis and pass it though Vector 4 a_v4
		Vector4 Matrix4::GetT()const
		{
			return Vector4(m_41, m_42, m_43, m_44);
		}

		//used to set the X axis to what is in Vector 3 a_v3
		void Matrix4::SetX(const Vector3& a_v3)
		{
			m[0][0] = a_v3.x;
			m[0][1] = a_v3.y;
			m[0][2] = a_v3.z;
			m[0][3] = 0;
		}

		//used to set the Y axis to what is in Vector 3 a_v3
		void Matrix4::SetY(const Vector3& a_v3)
		{
			m[1][0] = a_v3.x;
			m[1][1] = a_v3.y;
			m[1][2] = a_v3.z;
			m[1][3] = 0;
		}

		//used to set the Z axis to what is in Vector 3 a_v3
		void Matrix4::SetZ(const Vector3& a_v3)
		{
			m[2][0] = a_v3.x;
			m[2][1] = a_v3.y;
			m[2][2] = a_v3.z;
			m[2][3] = 0;
		}

		//used to set the T axis to what is in Vector 3 a_v3
		void Matrix4::SetT(const Vector3& a_v3)
		{
			m[3][0] = a_v3.x;
			m[3][1] = a_v3.y;
			m[3][2] = a_v3.z;
			m[3][3] = 1;
		}

		//Retrieve X axis and pass it though Vector 3 a_v3
		void Matrix4::GetX(Vector3& a_v3)const
		{
			a_v3.x = m[0][0];
			a_v3.y = m[0][1];
			a_v3.z = m[0][2];
			//= m[3][3];
		}

		//Retrieve Y axis and pass it though Vector 3 a_v3
		void Matrix4::GetY(Vector3& a_v3)const
		{
			a_v3.x = m[1][0];
			a_v3.y = m[1][1];
			a_v3.z = m[1][2];
		}

		//Retrieve Z axis and pass it though Vector 3 a_v3
		void Matrix4::GetZ(Vector3& a_v3)const
		{
			a_v3.x = m[2][0];
			a_v3.y = m[2][1];
			a_v3.z = m[2][2];
		}

		//Retrieve T axis and pass it though Vector 3 a_v3
		void Matrix4::GetT(Vector3& a_v3)const
		{
			a_v3.x = m[3][0];
			a_v3.y = m[3][1];
			a_v3.z = m[3][2];
		}

		//set column values to what is passed though by Vector 3 vCol

		void Matrix4::SetCol(int col, const Vector3& vCol)
		{
			m[0][col] = vCol.x;
			m[1][col] = vCol.y;
			m[2][col] = vCol.z;
			m[3][col] = 0;
		}

		//set column values to what is passed though by Vector 4 vCol
		void Matrix4::SetCol(int iCol, const Vector4& vCol)
		{
			m[0][iCol] = vCol.x;
			m[1][iCol] = vCol.y;
			m[2][iCol] = vCol.z;
			m[3][iCol] = vCol.w;
		}

		//get column values from Matrix 4 then pass them though Vector 3 vCol
		void Matrix4::GetCol(int iCol, Vector3& vCol) const
		{
			vCol.Set(m[0][iCol], m[1][iCol], m[2][iCol]);
		}

		//get column values from Matrix 4 then pass them though Vector 4 vCol
		void Matrix4::GetCol(int iCol, Vector4& vCol) const
		{
			vCol.Set(m[0][iCol], m[1][iCol], m[2][iCol], m[3][iCol]);
		}

		//set row values of Matrix 4 from values passed though from Vector 3 vRow
		void Matrix4::SetRow(int iRow, const Vector3& vRow)
		{
			m[iRow][0] = vRow.x;
			m[iRow][1] = vRow.y;
			m[iRow][2] = vRow.z;
			m[iRow][3] = 0.f;
		}

		//set row values of Matrix 4 from values passed though from Vector 4 vRow
		void Matrix4::SetRow(int iRow, const Vector4& vRow)
		{
			m[iRow][0] = vRow.x;
			m[iRow][1] = vRow.y;
			m[iRow][2] = vRow.z;
			m[iRow][3] = vRow.w;
		}

		//get row values from Matrix 4 then pass them though Vector 3 vCol
		void Matrix4::GetRow(int iRow, Vector3& vRow) const
		{
			vRow.Set(m[iRow][0], m[iRow][1], m[iRow][2]);
		}

		//get row values from Matrix 4 then pass them though Vector 4 vCol
		void Matrix4::GetRow(int iRow, Vector4& vRow) const
		{
			vRow.Set(m[iRow][0], m[iRow][1], m[iRow][2], m[iRow][3]);
		}

		//if Matrix 4is equal to Matrix 4 a_m4 then return true, all values must be the same
		//e.g: m_11 must be the same as a_m4.m_11
		bool Matrix4::operator==(const Matrix4& a_m4) const
		{
			return ((m_11 == a_m4.m_11) && (m_12 == a_m4.m_12) && (m_13 == a_m4.m_13) && (m_14 == a_m4.m_14) &&
				(m_21 == a_m4.m_21) && (m_22 == a_m4.m_22) && (m_23 == a_m4.m_23) && (m_24 == a_m4.m_24) &&
				(m_31 == a_m4.m_31) && (m_32 == a_m4.m_32) && (m_33 == a_m4.m_33) && (m_34 == a_m4.m_34) &&
				(m_41 == a_m4.m_41) && (m_42 == a_m4.m_42) && (m_43 == a_m4.m_43) && (m_44 == a_m4.m_44));
		}

		//if Matrix 4 is not equal to Matrix 4 a_m4, return true
		//e.g: if any one value is not the same return true, if all value are the same bu m_33 is not equal to a_m4.m_33 return true
		bool Matrix4::operator!=(const Matrix4& a_m4) const
		{
			return ((m_11 != a_m4.m_11) || (m_12 != a_m4.m_12) || (m_13 != a_m4.m_13) || (m_14 != a_m4.m_14) ||
				(m_21 != a_m4.m_21) || (m_22 != a_m4.m_22) || (m_23 != a_m4.m_23) || (m_24 != a_m4.m_24) ||
				(m_31 != a_m4.m_31) || (m_32 != a_m4.m_32) || (m_33 != a_m4.m_33) || (m_34 != a_m4.m_34) ||
				(m_41 != a_m4.m_41) || (m_42 != a_m4.m_42) || (m_43 != a_m4.m_43) || (m_44 != a_m4.m_44));
		}

		//Add Matrix 4 to Matrix 4 a_m4, return Matrix 4
		const Matrix4 Matrix4::operator+(const Matrix4& a_m4) const
		{
			return Matrix4(m_11 + a_m4.m_11, m_12 + a_m4.m_12, m_13 + a_m4.m_13, m_14 + a_m4.m_14,
				m_21 + a_m4.m_21, m_22 + a_m4.m_22, m_23 + a_m4.m_23, m_24 + a_m4.m_24,
				m_31 + a_m4.m_31, m_32 + a_m4.m_32, m_33 + a_m4.m_33, m_34 + a_m4.m_34,
				m_41 + a_m4.m_41, m_42 + a_m4.m_42, m_43 + a_m4.m_43, m_44 + a_m4.m_44);
		}

		//Add Matrix 4 a_m4 to Current Matrix 4, 
		const Matrix4& Matrix4::operator+=(const Matrix4& a_m4)
		{
			m_11 += a_m4.m_11;		m_12 += a_m4.m_12;		m_13 += a_m4.m_13;		m_14 += a_m4.m_14;

			m_21 += a_m4.m_21;		m_22 += a_m4.m_22;		m_23 += a_m4.m_23;		m_24 += a_m4.m_24;

			m_31 += a_m4.m_31;		m_32 += a_m4.m_32;		m_33 += a_m4.m_33;		m_34 += a_m4.m_34;

			m_41 += a_m4.m_41;		m_42 += a_m4.m_42;		m_43 += a_m4.m_43;		m_44 += a_m4.m_44;
			return *this;
		}

		//subtract Matrix 4 a_m4 from Matrix 4, returns Matrix 4 
		const Matrix4 Matrix4::operator-(const Matrix4& a_m4) const
		{
			return Matrix4(m_11 - a_m4.m_11, m_12 - a_m4.m_12, m_13 - a_m4.m_13, m_14 - a_m4.m_14,
				m_21 - a_m4.m_21, m_22 - a_m4.m_22, m_23 - a_m4.m_23, m_24 - a_m4.m_24,
				m_31 - a_m4.m_31, m_32 - a_m4.m_32, m_33 - a_m4.m_33, m_34 - a_m4.m_34,
				m_41 - a_m4.m_41, m_42 - a_m4.m_42, m_43 - a_m4.m_43, m_44 - a_m4.m_44);
		}

		//subtract Matrix 4 a_m4 from current Matrix 4 a_m4
		const Matrix4& Matrix4::operator-=(const Matrix4& a_m4)
		{
			m_11 -= a_m4.m_11;		m_12 -= a_m4.m_12;		m_13 -= a_m4.m_13;		m_14 -= a_m4.m_14;

			m_21 -= a_m4.m_21;		m_22 -= a_m4.m_22;		m_23 -= a_m4.m_23;		m_24 -= a_m4.m_24;

			m_31 -= a_m4.m_31;		m_32 -= a_m4.m_32;		m_33 -= a_m4.m_33;		m_34 -= a_m4.m_34;

			m_41 -= a_m4.m_41;		m_42 -= a_m4.m_42;		m_43 -= a_m4.m_43;		m_44 -= a_m4.m_44;
			return *this;
		}

		//multiply Matrix 4 by a_fScalar, return Matrix 4
		const Matrix4 Matrix4::operator*(const float a_fScalar) const
		{
			return Matrix4(m_11 * a_fScalar, m_12 * a_fScalar, m_13 * a_fScalar, m_14 * a_fScalar,
				m_21 * a_fScalar, m_22 * a_fScalar, m_23 * a_fScalar, m_24 * a_fScalar,
				m_31 * a_fScalar, m_32 * a_fScalar, m_33 * a_fScalar, m_34 * a_fScalar,
				m_41 * a_fScalar, m_42 * a_fScalar, m_43 * a_fScalar, m_44 * a_fScalar);
		}

		//multiply Matrix 4 by a_fScalar
		const Matrix4& Matrix4::operator*=(const float a_fScalar)
		{
			m_11 *= a_fScalar;		m_12 *= a_fScalar;		m_13 *= a_fScalar;	m_14 *= a_fScalar;
			m_21 *= a_fScalar;		m_22 *= a_fScalar;		m_23 *= a_fScalar;	m_24 *= a_fScalar;
			m_31 *= a_fScalar;		m_32 *= a_fScalar;		m_33 *= a_fScalar;	m_34 *= a_fScalar;
			m_41 *= a_fScalar;		m_42 *= a_fScalar;		m_43 *= a_fScalar;	m_44 *= a_fScalar;
			return *this;
		}

		//multiply Matrix 4 by Vector 3 a_v3, returns Vector 3 
		//X axis becomes X position
		const Vector3 Matrix4::operator*(const Vector3& a_v3) const
		{
			return Vector3(a_v3.x * m[0][0] + a_v3.y * m[0][1] + a_v3.z * m[0][2],
				a_v3.x * m[1][0] + a_v3.y * m[1][1] + a_v3.z * m[1][2],
				a_v3.x * m[2][0] + a_v3.y * m[2][1] + a_v3.z * m[2][2]);
		}

		//multiply Matrix 4 by Vector 4 a_v4, returns Vector 4
		//X axis becomes x position
		const Vector4 Matrix4::operator*(const Vector4& a_v4) const
		{
			return Vector4(a_v4.x * m[0][0] + a_v4.y + m[0][1] + a_v4.z * m[0][2] + a_v4.w * m[0][3],
				a_v4.x * m[1][0] + a_v4.y + m[1][1] + a_v4.z * m[1][2] + a_v4.w * m[1][3],
				a_v4.x * m[2][0] + a_v4.y + m[2][1] + a_v4.z * m[2][2] + a_v4.w * m[2][3],
				a_v4.x * m[3][0] + a_v4.y + m[3][1] + a_v4.z * m[3][2] + a_v4.w * m[3][3]);
		}

		//used to multiply Matrix4 by Matrix4 a_m4, returns a Matrix 4 
		const Matrix4 Matrix4::operator*(const Matrix4& a_m4) const
		{
			return Matrix4(
				m_11 * a_m4.m_11 + m_12 * a_m4.m_21 + m_13 * a_m4.m_31 + m_14 * a_m4.m_41,
				m_11 * a_m4.m_12 + m_12 * a_m4.m_22 + m_13 * a_m4.m_32 + m_14 * a_m4.m_42,
				m_11 * a_m4.m_13 + m_12 * a_m4.m_23 + m_13 * a_m4.m_33 + m_14 * a_m4.m_43,
				m_11 * a_m4.m_14 + m_12 * a_m4.m_24 + m_13 * a_m4.m_34 + m_14 * a_m4.m_44,

				m_21 * a_m4.m_11 + m_22 * a_m4.m_21 + m_23 * a_m4.m_31 + m_24 * a_m4.m_41,
				m_21 * a_m4.m_12 + m_22 * a_m4.m_22 + m_23 * a_m4.m_32 + m_24 * a_m4.m_42,
				m_21 * a_m4.m_13 + m_22 * a_m4.m_23 + m_23 * a_m4.m_33 + m_24 * a_m4.m_43,
				m_21 * a_m4.m_14 + m_22 * a_m4.m_24 + m_23 * a_m4.m_34 + m_24 * a_m4.m_44,

				m_31 * a_m4.m_11 + m_32 * a_m4.m_21 + m_33 * a_m4.m_31 + m_34 * a_m4.m_41,
				m_31 * a_m4.m_12 + m_32 * a_m4.m_22 + m_33 * a_m4.m_32 + m_34 * a_m4.m_42,
				m_31 * a_m4.m_13 + m_32 * a_m4.m_23 + m_33 * a_m4.m_33 + m_34 * a_m4.m_43,
				m_31 * a_m4.m_14 + m_32 * a_m4.m_24 + m_33 * a_m4.m_34 + m_34 * a_m4.m_44,

				m_41 * a_m4.m_11 + m_42 * a_m4.m_21 + m_43 * a_m4.m_31 + m_44 * a_m4.m_41,
				m_41 * a_m4.m_12 + m_42 * a_m4.m_22 + m_43 * a_m4.m_32 + m_44 * a_m4.m_42,
				m_41 * a_m4.m_13 + m_42 * a_m4.m_23 + m_43 * a_m4.m_33 + m_44 * a_m4.m_43,
				m_41 * a_m4.m_14 + m_42 * a_m4.m_24 + m_43 * a_m4.m_34 + m_44 * a_m4.m_44
			);
		}


		//used to multiply Matrix4 by Matrix4 a_m4, does not return a Matrix4
		const Matrix4& Matrix4::operator*=(const Matrix4& a_m4)
		{
			m_11 *= a_m4.m_11;	m_12 *= a_m4.m_12;	m_13 *= a_m4.m_13;	m_14 *= a_m4.m_14;

			m_21 *= a_m4.m_21; m_22 *= a_m4.m_22; m_23 *= a_m4.m_23;	m_24 *= a_m4.m_24;

			m_31 *= a_m4.m_31;	m_32 *= a_m4.m_32;	m_33 *= a_m4.m_33;	m_34 *= a_m4.m_34;

			m_41 *= a_m4.m_41;	m_42 *= a_m4.m_43;	m_23 *= a_m4.m_43;	m_44 *= a_m4.m_44;
			return *this;
		}

		//Transposes the Matrix 4
		void Matrix4::Transpose()
		{
			float k = m_12;
			m_12 = m_21;
			m_21 = k;

			k = m_13;
			m_13 = m_31;
			m_31 = k;

			k = m_14;
			m_14 = m_41;
			m_44 = k;

			k = m_23;
			m_23 = m_32;
			m_32 = k;

			k = m_24;
			m_24 = m_42;
			m_42 = k;

			k = m_34;
			m_34 = m_43;
			m_43 = k;
		}

		//Transposes the Matrix 4, then return it though "Matrix4 &mat"
		void Matrix4::GetTranspose(Matrix4& mat)
		{
			Transpose();

			mat.m_11 = m_11;	 mat.m_12 = m_12;	mat.m_13 = m_13;	mat.m_14 = m_14;

			mat.m_21 = m_21;	 mat.m_22 = m_22;	mat.m_23 = m_23;	mat.m_24 = m_24;

			mat.m_31 = m_31;	 mat.m_32 = m_32;	mat.m_33 = m_33;	mat.m_34 = m_34;

			mat.m_41 = m_41;	 mat.m_42 = m_42;	mat.m_43 = m_43;	mat.m_44 = m_44;
		}

		void Matrix4::Scale(const Vector3& a_v3)
		{
			m_11* a_v3.x, m_12* a_v3.x, m_13* a_v3.x,

				m_21* a_v3.y, m_22* a_v3.y, m_23* a_v3.y,

				m_31* a_v3.z, m_32* a_v3.z, m_33* a_v3.z;
		}

		void Matrix4::Scale(float a_fScalar)
		{
			m_11* a_fScalar, m_12* a_fScalar, m_13* a_fScalar, m_14* a_fScalar,

				m_21* a_fScalar, m_22* a_fScalar, m_23* a_fScalar, m_24* a_fScalar,

				m_31* a_fScalar, m_32* a_fScalar, m_33* a_fScalar, m_34* a_fScalar;

			m_41* a_fScalar, m_42* a_fScalar, m_43* a_fScalar, m_44* a_fScalar;
		}


		float Matrix4::Determinant()const
		{
			float fA = m_11 * (m_22 * (m_33 * m_44 - m_34 * m_43) +
				m_23 * (m_34 * m_42 - m_32 * m_44) +
				m_24 * (m_32 * m_43 - m_33 * m_42));

			float fB = m_12 * (m_21 * (m_33 * m_44 - m_34 * m_43) +
				m_23 * (m_34 * m_41 - m_31 * m_44) +
				m_24 * (m_31 * m_43 - m_33 * m_41));

			float fC = m_13 * (m_21 * (m_32 * m_44 - m_34 * m_42) +
				m_22 * (m_34 * m_41 - m_31 * m_44) +
				m_24 * (m_31 * m_42 - m_32 * m_41));

			float fD = m_14 * (m_21 * (m_32 * m_43 - m_33 * m_42) +
				m_22 * (m_33 * m_41 - m_31 * m_43) +
				m_23 * (m_31 * m_42 - m_32 * m_41));

			return  fA - fB + fC - fD;
		}
		float Matrix4::Determinant3() const
		{
			return	(m_11 * (m_22 * m_33 - m_23 * m_32) +
				m_12 * (m_23 * m_31 - m_21 * m_33) +
				m_13 * (m_21 * m_32 - m_22 * m_31));
		}


		//\===========================================================================================	
		// Inverse Matrix Functions
		//\ Calculates the determinant. This expresion could not be very accurate
		//\ when there are big coefficents in the matrix. To be improved if required.
		//\===========================================================================================
		bool Matrix4::Inverse()
		{
			float fDeterminant = Determinant();
			if (fDeterminant != 0.0f)
			{
				const float fInvDet = 1 / fDeterminant;

				Matrix4 mat = *this;
				m_11 = (mat.m_22 * mat.m_33 - mat.m_23 * mat.m_32) * fInvDet;
				m_12 = (mat.m_13 * mat.m_32 - mat.m_12 * mat.m_33) * fInvDet;
				m_13 = (mat.m_12 * mat.m_23 - mat.m_13 * mat.m_22) * fInvDet;
				m_14 = 0.f;

				m_21 = (mat.m_23 * mat.m_31 - mat.m_21 * mat.m_33) * fInvDet;
				m_22 = (mat.m_11 * mat.m_33 - mat.m_13 * mat.m_31) * fInvDet;
				m_23 = (mat.m_13 * mat.m_21 - mat.m_11 * mat.m_23) * fInvDet;
				m_24 = 0.f;

				m_31 = (mat.m_21 * mat.m_32 - mat.m_22 * mat.m_31) * fInvDet;
				m_32 = (mat.m_12 * mat.m_31 - mat.m_11 * mat.m_32) * fInvDet;
				m_33 = (mat.m_11 * mat.m_22 - mat.m_12 * mat.m_21) * fInvDet;
				m_34 = 0.f;

				m_41 = (mat.m_21 * (mat.m_33 * mat.m_42 - mat.m_32 * mat.m_43) +
					mat.m_22 * (mat.m_31 * mat.m_43 - mat.m_33 * mat.m_41) +
					mat.m_23 * (mat.m_32 * mat.m_41 - mat.m_31 * mat.m_42)) * fInvDet;
				m_42 = (mat.m_11 * (mat.m_32 * mat.m_43 - mat.m_33 * mat.m_42) +
					mat.m_12 * (mat.m_33 * mat.m_41 - mat.m_31 * mat.m_43) +
					mat.m_13 * (mat.m_31 * mat.m_42 - mat.m_32 * mat.m_41)) * fInvDet;
				m_43 = (mat.m_11 * (mat.m_23 * mat.m_42 - mat.m_22 * mat.m_43) +
					mat.m_12 * (mat.m_21 * mat.m_43 - mat.m_23 * mat.m_41) +
					mat.m_13 * (mat.m_22 * mat.m_41 - mat.m_21 * mat.m_42)) * fInvDet;
				m_44 = 1.0f;

				return true;
			}
			return false;
		}

		bool Matrix4::GetInverse(Matrix4& a_m4) const
		{
			Matrix4 mm = a_m4;
			return false;
		}

		void Matrix4::RotateX(float fAngle)
		{
			//use the created Matrix4 is CreateRotateX and times it by Matrix4
			//this means that you do not need to make a Matrix4 then rotate it then times it by a position
			//Matrix
			*this = CreateRotateX(fAngle) * *this;
		}

		void Matrix4::RotateY(float fAngle)
		{
			*this = CreateRotateY(fAngle) * *this;;
		}
		void Matrix4::RotateZ(float fAngle)
		{
			*this = CreateRotateZ(fAngle) * *this;
		}

		Matrix4 CreateRotateX(float fAngle)
		{
			Matrix4 m4;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m4.m_11 = 1.0f, m4.m_12 = 0.f, m4.m_13 = 0.f, m4.m_14 = 0.f;
			m4.m_21 = 0.f, m4.m_22 = c, m4.m_23 = s, m4.m_24 = 0.f;
			m4.m_31 = 0.f, m4.m_32 = -s, m4.m_33 = c, m4.m_34 = 0.f;
			m4.m_41 = 0.f, m4.m_42 = 0.f, m4.m_43 = 0.f, m4.m_44 = 1.f;

			return m4;
		}

		Matrix4 CreateRotateY(float fAngle)
		{
			Matrix4 m4;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m4.m_11 = c, m4.m_12 = 0.f, m4.m_13 = -s, m4.m_14 = 0.f;
			m4.m_21 = 0.f, m4.m_22 = 1.f, m4.m_23 = 0.f, m4.m_24 = 0.f;
			m4.m_31 = s, m4.m_32 = 0.f, m4.m_33 = c, m4.m_34 = 0.f;
			m4.m_41 = 0.f, m4.m_42 = 0.f, m4.m_43 = 0.f, m4.m_44 = 1.f;

			return m4;
		}

		Matrix4 CreateRotateZ(float fAngle)
		{
			Matrix4 m4;

			float c = cosf(fAngle);
			float s = sinf(fAngle);

			m4.m_11 = c, m4.m_12 = s, m4.m_13 = 0.f, m4.m_14 = 0.f;
			m4.m_21 = -s, m4.m_22 = c, m4.m_23 = 0.f, m4.m_24 = 0.f;
			m4.m_31 = 0.f, m4.m_32 = 0.f, m4.m_33 = 1.f, m4.m_34 = 0.f;
			m4.m_41 = 0.f, m4.m_42 = 0.f, m4.m_43 = 0.f, m4.m_44 = 1.f;

			return m4;
		}

		void Matrix4::Identity()
		{
			m_11 = 1.f, m_12 = 0.f, m_13 = 0.f, m_14 = 0.f;
			m_21 = 0.f, m_22 = 1.f, m_23 = 0.f, m_24 = 0.f;
			m_31 = 0.f, m_32 = 0.f, m_33 = 1.f, m_34 = 0.f;
			m_41 = 0.f, m_42 = 0.f, m_43 = 0.f, m_44 = 1.f;
		}

		void Matrix4::Zero()
		{
			m_11 = 0.f, m_12 = 0.f, m_13 = 0.f, m_14 = 0.f;
			m_21 = 0.f, m_22 = 0.f, m_23 = 0.f, m_24 = 0.f;
			m_31 = 0.f, m_32 = 0.f, m_33 = 0.f, m_34 = 0.f;
			m_41 = 0.f, m_42 = 0.f, m_43 = 0.f, m_44 = 0.f;
		}

		//\This is used to change the x and y position of the matrix4, this allows objects to be moved. This function is for 2D games 
		void Matrix4::Translate(const Vector2 a_v2)
		{
			m_41 += a_v2.x;
			m_42 += a_v2.y;
		}

		//\This is used to change the x, y and z position of the matrix4, this allows objects to be moved. This function is for 3D games 
		void Matrix4::Translate(const Vector3 a_v3)
		{
			m_41 += a_v3.x;
			m_42 += a_v3.y;
			m_43 += a_v3.z;
		}

		//used to see which way the Y is facing then changing the translation based on that
		void Matrix4::MoveForwards(const float a_f)
		{
			Vector4 v4 = GetY();
			Translate(Vector2(v4.x, v4.y) * a_f);
		}

		void Matrix4::MoveBackwards(const float a_f)
		{
			Vector4 v4 = GetY();
			Translate(Vector2(v4.x, v4.y) * -a_f);
		}
	}
}