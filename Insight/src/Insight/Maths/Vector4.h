#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Maths
	{
		class IS_API Vector4
		{
		public:
			Vector4();
			Vector4(float a_x, float a_y, float a_z, float a_w);
			Vector4(const Vector4& a_v3);

			~Vector4();
			//\===========================================================================================
			//\Casting operators
			//\===========================================================================================
			operator float* ();
			operator const			float* ();

			//\===========================================================================================
			//\ Accessor Operators
			//\===========================================================================================
			void					Get(float& a_x, float& a_y, float& a_z, float& a_w) const;
			void					Set(const float& a_x, const float& a_y, const float& a_z, const float& a_w);
			float& operator[](const int& a_index);
			Vector4& operator=(const Vector4& vector);
			Vector4& operator=(const Vector4&& vector) noexcept;

			//\===========================================================================================
			// Equivalence operators
			//\===========================================================================================
			bool					operator ==			(const Vector4& a_v3) const;
			bool					operator !=			(const Vector4& a_v3) const;

			//\===========================================================================================
			//\ Neg operator
			//\===========================================================================================
			const	Vector4			operator-			() const;

			//\===========================================================================================
			//\ Overload Operators for Vector4 addition
			//\===========================================================================================
			Vector4					operator +				(const float a_scalar) const;
			Vector4					operator +				(const Vector4& a_v3) const;
			Vector4& operator +=				(const float a_scalar);
			Vector4& operator +=				(const Vector4& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector4 subtraction
			//\===========================================================================================
			Vector4					operator -				(const float a_scalar) const;
			Vector4					operator -				(const Vector4& a_v3) const;
			Vector4& operator -=				(const float a_scalar);
			Vector4& operator -=				(const Vector4& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector4 multiplication
			//\===========================================================================================
			Vector4					operator *				(const float a_scalar) const;
			Vector4					operator *				(const Vector4& a_v3) const;
			Vector4& operator *=				(const float a_scalar);
			Vector4& operator *=				(const Vector4& a_v3);

			//\===========================================================================================
			//\ Overload Operators for Vector4 division
			//\===========================================================================================
			Vector4					operator /			    (const float a_scalar) const;
			Vector4					operator /				(const Vector4& a_v3) const;
			Vector4& operator /=				(const float a_scalar);
			Vector4& operator /=				(const Vector4& a_v3);

			//\===========================================================================================
			//\ Vector4 Length/Magnitude Functions
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
			friend  float			Distance(const Vector4& a_v3A, const Vector4& a_v3B);
			friend  float			DistanceSquared(const Vector4& a_v3A, const Vector4& a_v3B);
			//\===========================================================================================
			//\ Dot Product Functionality
			//\===========================================================================================
			float                   Dot(const Vector4& a_v3) const;
			Vector4					CrossProduct(const Vector4& a_v3) const;
			friend  float			Dot(const Vector4& vecA, const Vector4& a_v3B);
			//\===========================================================================================
			//\ Normalization
			//\===========================================================================================
			bool                   IsUnit() const;
			float* Normalise();
			const Vector4          GetUnit() const;
			//\===========================================================================================
			//\ Get Perpendicular
			//\===========================================================================================
			Vector4                GetPerpendicular(const Vector4& a_v4) const;
			//\===========================================================================================
			//\ Transformation Functions
			//\===========================================================================================
			void                    RotateX(float fAngle);
			void                    RotateY(float fAngle);
			void                    RotateZ(float fAngle);
			//	void                    Project(float fAngle, float fDistance);
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
					float w;
				};
				struct
				{
					float r;
					float g;
					float b;
					float a;
				};
				struct
				{
					float i[4];
				};
			};
		};
	}
}