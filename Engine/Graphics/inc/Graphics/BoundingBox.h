/*
Copyright(c) 2016-2022 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Graphics/Defines.h"

#include "Maths/Vector3.h"
#include "Maths/Matrix4.h"

#include <cmath>

namespace Insight
{
	namespace Graphics
	{
		struct Vertex;

		enum class Intersection
		{
			Outside,
			Inside,
			Intersects
		};

		class IS_GRAPHICS BoundingBox
		{
		public:
			/// Construct with zero size.
			BoundingBox();

			/// Construct from minimum and maximum vectors.
			BoundingBox(const Maths::Vector3& min, const Maths::Vector3& max);

			/// Construct from points
			BoundingBox(const Maths::Vector3* vertices, const uint32_t point_count);

			/// Construct from vertices
			BoundingBox(float* positions, const int positionsStride, const uint64_t positionsCount);

			~BoundingBox() = default;

			/// Assign from bounding box
			BoundingBox& operator =(const BoundingBox& rhs) = default;

			/// Returns the center
			Maths::Vector3 GetCenter() const { return (m_max + m_min) * 0.5f; }

			/// Returns the size
			Maths::Vector3 GetSize() const { return m_max - m_min; }

			// Get the radius
			float GetRadius() const;

			/// Returns extents
			Maths::Vector3 GetExtents() const { return (m_max - m_min) * 0.5f; }

			/// Test if a point is inside
			Intersection IsInside(const Maths::Vector3& point) const;

			/// Test if a bounding box is inside
			Intersection IsInside(const BoundingBox& box) const;

			/// Returns a transformed bounding box
			BoundingBox Transform(const Maths::Matrix4& transform) const;

			/// Merge with another bounding box
			void Merge(const BoundingBox& box);

			const Maths::Vector3& GetMin() const { return m_min; }
			const Maths::Vector3& GetMax() const { return m_max; }

			bool Defined() const { return m_min.x != INFINITY; }

			static const BoundingBox Zero;

		private:
			Maths::Vector3 m_min;
			Maths::Vector3 m_max;
		};
	}
}