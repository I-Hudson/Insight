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

#include "Graphics/BoundingBox.h"
#include "Graphics/Mesh.h"

namespace Insight
{
	namespace Graphics
	{
        const BoundingBox BoundingBox::Zero(glm::vec3(0), glm::vec3(0));

        BoundingBox::BoundingBox()
        {
            m_min = glm::vec3((float)UINT32_MAX);
            m_max = glm::vec3((float)INT32_MIN);
        }

        BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max)
        {
            this->m_min = min;
            this->m_max = max;
        }

        BoundingBox::BoundingBox(const glm::vec3* points, const uint32_t point_count)
        {
            m_min = glm::vec3((float)UINT32_MAX);
            m_max = glm::vec3((float)INT32_MIN);

            for (uint32_t i = 0; i < point_count; i++)
            {
                m_max.x = glm::max(m_max.x, points[i].x);
                m_max.y = glm::max(m_max.y, points[i].y);
                m_max.z = glm::max(m_max.z, points[i].z);

                m_min.x = glm::min(m_min.x, points[i].x);
                m_min.y = glm::min(m_min.y, points[i].y);
                m_min.z = glm::min(m_min.z, points[i].z);
            }
        }

        BoundingBox::BoundingBox(const Vertex* vertices, const uint32_t vertex_count)
        {
            m_min = glm::vec3((float)UINT32_MAX);
            m_max = glm::vec3((float)INT32_MIN);

            for (uint32_t i = 0; i < vertex_count; ++i)
            {
                m_max.x = glm::max(m_max.x, vertices[i].Position.x);
                m_max.y = glm::max(m_max.y, vertices[i].Position.y);
                m_max.z = glm::max(m_max.z, vertices[i].Position.z);

                m_min.x = glm::min(m_min.x, vertices[i].Position.x);
                m_min.y = glm::min(m_min.y, vertices[i].Position.y);
                m_min.z = glm::min(m_min.z, vertices[i].Position.z);
            }
        }

        Intersection BoundingBox::IsInside(const glm::vec3& point) const
        {
            if (point.x < m_min.x || point.x > m_max.x ||
                point.y < m_min.y || point.y > m_max.y ||
                point.z < m_min.z || point.z > m_max.z)
            {
                return Intersection::Outside;
            }
            else
            {
                return Intersection::Inside;
            }
        }

        Intersection BoundingBox::IsInside(const BoundingBox& box) const
        {
            if (box.m_max.x < m_min.x || box.m_min.x > m_max.x ||
                box.m_max.y < m_min.y || box.m_min.y > m_max.y ||
                box.m_max.z < m_min.z || box.m_min.z > m_max.z)
            {
                return Intersection::Outside;
            }
            else if (
                box.m_min.x < m_min.x || box.m_max.x > m_max.x ||
                box.m_min.y < m_min.y || box.m_max.y > m_max.y ||
                box.m_min.z < m_min.z || box.m_max.z > m_max.z)
            {
                return Intersection::Intersects;
            }
            else
            {
                return Intersection::Inside;
            }
        }

        BoundingBox BoundingBox::Transform(const glm::mat4& transform) const
        {
            const glm::vec3 center_new = transform * glm::vec4(GetCenter(), 1);
            const glm::vec3 extent_old = GetExtents();
            const glm::vec3 extend_new = glm::vec3
            (
                glm::abs(transform[0][0]) * extent_old.x + glm::abs(transform[1][0]) * extent_old.y + glm::abs(transform[2][0]) * extent_old.z,
                glm::abs(transform[0][1]) * extent_old.x + glm::abs(transform[1][1]) * extent_old.y + glm::abs(transform[2][1]) * extent_old.z,
                glm::abs(transform[0][2]) * extent_old.x + glm::abs(transform[1][2]) * extent_old.y + glm::abs(transform[2][2]) * extent_old.z
            );

            return BoundingBox(center_new - extend_new, center_new + extend_new);
        }

        void BoundingBox::Merge(const BoundingBox& box)
        {
            m_min.x = glm::min(m_min.x, box.m_min.x);
            m_min.y = glm::min(m_min.y, box.m_min.y);
            m_min.z = glm::min(m_min.z, box.m_min.z);
            m_max.x = glm::max(m_max.x, box.m_max.x);
            m_max.y = glm::max(m_max.x, box.m_max.x);
            m_max.z = glm::max(m_max.x, box.m_max.x);
        }
	}
}