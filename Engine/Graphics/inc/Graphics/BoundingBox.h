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
#include "Frustum.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Insight
{
	namespace Graphics
	{
		struct Vertex;

		class IS_GRAPHICS BoundingBox
		{
		public:
			/// Construct with zero size.
			BoundingBox();

			/// Construct from minimum and maximum vectors.
			BoundingBox(const glm::vec3& min, const glm::vec3& max);

			/// Construct from points
			BoundingBox(const glm::vec3* vertices, const uint32_t point_count);

			/// Construct from vertices
			BoundingBox(const Vertex* vertices, const uint32_t vertex_count);

			~BoundingBox() = default;

			/// Assign from bounding box
			BoundingBox& operator =(const BoundingBox& rhs) = default;

			/// Returns the center
			glm::vec3 GetCenter() const { return (m_max + m_min) * 0.5f; }

			/// Returns the size
			glm::vec3 GetSize() const { return m_max - m_min; }

			/// Returns extents
			glm::vec3 GetExtents() const { return (m_max - m_min) * 0.5f; }

			/// Test if a point is inside
			Intersection IsInside(const glm::vec3& point) const;

			/// Test if a bounding box is inside
			Intersection IsInside(const BoundingBox& box) const;

			/// Returns a transformed bounding box
			BoundingBox Transform(const glm::mat4& transform) const;

			/// Merge with another bounding box
			void Merge(const BoundingBox& box);

			const glm::vec3& GetMin() const { return m_min; }
			const glm::vec3& GetMax() const { return m_max; }

			bool Defined() const { return m_min.x != INFINITY; }

			static const BoundingBox Zero;

		private:
			glm::vec3 m_min;
			glm::vec3 m_max;
		};
	}
}