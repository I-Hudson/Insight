#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Maths
	{
		class IS_API Vector2
		{
		public:
			//\===========================================================================================
			//\ Constants
			//\===========================================================================================
			Vector2();
			Vector2(float a_x, float a_y);
			explicit Vector2(const float* a_i);
			Vector2(const Vector2& a_v2);

			//\===========================================================================================
			//\ Destructor
			//\===========================================================================================
			~Vector2();

			//\===========================================================================================
			//\Casting operators
			//\===========================================================================================
			operator float* ();
			operator const			float* ();

			//\===========================================================================================
			//\ Accessor Operators
			//\===========================================================================================
			void					Get(float& a_x, float& a_y) const;
			void					Set(const float& a_x, const float& a_y);
			Vector2&				operator=(const Vector2& vector);
			Vector2&				operator=(const Vector2&& vector) noexcept;

			//\===========================================================================================
			// Equivalence operators
			//\===========================================================================================
			bool					operator ==			(const Vector2& a_v2) const;
			bool					operator !=			(const Vector2& a_v2) const;

			//\===========================================================================================
			//\ Neg operator
			//\===========================================================================================
			const Vector2			operator-			() const;

			//\===========================================================================================
			//\ Overload Operators for Vector2 addition
			//\===========================================================================================
			Vector2					operator +				(const float a_scalar) const;
			Vector2					operator +				(const Vector2& a_v2) const;
			Vector2& operator +=				(const float a_scalar);
			Vector2& operator +=				(const Vector2& a_v2);

			//\===========================================================================================
			//\ Overload Operators for Vector2 subtraction
			//\===========================================================================================
			Vector2					operator -				(const float a_scalar) const;
			Vector2					operator -				(const Vector2& a_v2) const;
			Vector2& operator -=				(const float a_scalar);
			Vector2& operator -=				(const Vector2& a_v2);

			//\===========================================================================================
			//\ Overload Operators for Vector2 multiplication
			//\===========================================================================================
			Vector2					operator *				(const float a_scalar) const;
			Vector2					operator *				(const Vector2& a_v2) const;
			friend const Vector2    operator *				(const float a_scalar, const Vector2& a_v2);
			Vector2& operator *=				(const float a_scalar);
			Vector2& operator *=				(const Vector2& a_v2);

			//\===========================================================================================
			//\ Overload Operators for Vector2 division
			//\===========================================================================================
			Vector2					operator /			    (const float a_scalar) const;
			Vector2					operator /				(const Vector2& a_v2) const;
			Vector2& operator /=				(const float a_scalar);
			Vector2& operator /=				(const Vector2& a_v2);

			//\===========================================================================================
			//\ Vector2 Length/Magnitude Functions
			//\===========================================================================================
			float                   Length() const;
			float                   Magnitude() const;
			float                   LengthSquared() const;
			float                   MagnitudeSquared() const;

			//\===========================================================================================
			//\ Vector2 Bearing Functions
			//\===========================================================================================
			float Bearing()const;

			//\===========================================================================================
			//\ Distance
			//\===========================================================================================
			friend  float         Distance(const Vector2& a_v2A, const Vector2& a_v2B);
			friend  float         DistanceSquared(const Vector2& a_v2A, const Vector2& a_v2B);
			//\===========================================================================================
			//\ Dot Product Functionality
			//\===========================================================================================
			float                   Dot(const Vector2& a_v2) const;
			friend  float			Dot(const Vector2& vecA, const Vector2& a_v2B);
			//\===========================================================================================
			//\ Normalization
			//\===========================================================================================
			bool                   IsUnit() const;
			float* Normalise();
			const Vector2          GetUnit() const;
			//\===========================================================================================
			//\ Get Perpendicular
			//\===========================================================================================
			Vector2                GetPerpendicular() const;
			//\===========================================================================================
			//\ Transformation Functions
			//\===========================================================================================
			void                    Rotate(float fAngle);
			void                    Project(float fAngle, float fDistance);
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
				};
				struct
				{
					float u;
					float v;
				};
				struct
				{
					float i[2];
				};
			};
		};
	}
}
