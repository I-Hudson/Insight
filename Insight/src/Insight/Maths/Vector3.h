#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Maths
	{
		class IS_API Vector3
		{
		public:
			//\===========================================================================================
			//\ Constants
			//\===========================================================================================
			Vector3();
			Vector3(float a_x, float a_y, float a_z);
			Vector3(const Vector3& a_v3);

			//\===========================================================================================
			//\ Destructor
			//\===========================================================================================
			~Vector3();

			//\===========================================================================================
			//\Casting operators
			//\===========================================================================================
			operator float* ();
			operator const			float* ();

			//\===========================================================================================
			//\ Accessor Operators
			//\===========================================================================================
			void					Get(float& a_x, float& a_y, float& a_z) const;
			void					Set(const float& a_x, const float& a_y, const float& a_z);
			float& operator[](const int& a_index);
			Vector3& operator=(const Vector3& vector);
			Vector3& operator=(const Vector3&& vector) noexcept;

			//\===========================================================================================
			// Equivalence operators
			//\===========================================================================================
			bool					operator ==			(const Vector3& a_v3) const;
			bool					operator !=			(const Vector3& a_v3) const;

			//\===========================================================================================
			//\ Neg operator
			//\===========================================================================================
			const Vector3			operator-			() const;

			//\===========================================================================================
			//\ Overload Operators for Vector3 addition
			//\===========================================================================================
			Vector3					operator +				(const float a_scalar) const;
			Vector3					operator +				(const Vector3& a_v3) const;
			Vector3& operator +=				(const float a_scalar);
			Vector3& operator +=				(const Vector3& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector3 subtraction
			//\===========================================================================================
			Vector3					operator -				(const float a_scalar) const;
			Vector3					operator -				(const Vector3& a_v3) const;
			Vector3& operator -=				(const float a_scalar);
			Vector3& operator -=				(const Vector3& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector3 multiplication
			//\===========================================================================================
			Vector3					operator *				(const float a_scalar) const;
			Vector3					operator *				(const Vector3& a_v3) const;
			friend  const Vector3   operator *				(const float a_scalar, const Vector3& a_v3);
			Vector3& operator *=				(const float a_scalar);
			Vector3& operator *=				(const Vector3& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector3 division
			//\===========================================================================================
			Vector3					operator /			    (const float a_scalar) const;
			Vector3					operator /				(const Vector3& a_v3) const;
			Vector3& operator /=				(const float a_scalar);
			Vector3& operator /=				(const Vector3& a_v3);

			//\===========================================================================================
			//\ Vector3 Length/Magnitude Functions
			//\===========================================================================================
			float                   Length() const;
			float                   Magnitude() const;
			float                   LengthSquared() const;
			float                   MagnitudeSquared() const;

			//\===========================================================================================
			//\     A function to quickly shorten a vector to a desired length
			//\     This function has proven to be quite effective with camera controls
			//\===========================================================================================
			void                    Truncate(float a_fMaxLength);

			//\===========================================================================================
			//\ Distance
			//\===========================================================================================
			friend  float         Distance(const Vector3& a_v3A, const Vector3& a_v3B);
			friend  float         DistanceSquared(const Vector3& a_v3A, const Vector3& a_v3B);
			//\===========================================================================================
			//\ Dot Product Functionality
			//\===========================================================================================
			float                   Dot(const Vector3& a_v3) const;
			Vector3					CrossProduct(const Vector3& a_v3) const;
			friend  float			Dot(const Vector3& vecA, const Vector3& a_v3B);
			//\===========================================================================================
			//\ Normalization
			//\===========================================================================================
			bool                   IsUnit() const;
			float* Normalise();
			const Vector3          GetUnit() const;
			//\===========================================================================================
			//\ Get Perpendicular
			//\===========================================================================================
			Vector3                GetPerpendicular(const Vector3& a_v3) const;
			//\===========================================================================================
			//\ Transformation Functions
			//\===========================================================================================
			void                    RotateX(float fAngle);
			void                    RotateY(float fAngle);
			void                    RotateZ(float fAngle);
			//\===========================================================================================
			//\ Linear Interpolation and BiLinear Interpolation
			//\===========================================================================================
			friend   Vector3     Lerp(const Vector3& vecA, const Vector3& vecB, float t);
			friend   Vector3     BiLerp(const Vector3 vec[4], float fU, float fV);
			//\===========================================================================================
			// Other useful functions
			//\===========================================================================================
			void                    Zero();
			void                    One();
			float                   Sum() const;
			float                   Min() const;
			float                   Max() const;
			//\===========================================================================================

			//\===========================================================================================
			//\ Member Variables held in unnamed union for accessibility
			//\===========================================================================================
			union
			{
				struct
				{
					float x;
					float y;
					float z;
				};
				struct
				{
					float i[3];
				};
			};
		};
	}
}