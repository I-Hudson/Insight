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
#include "Graphics/BoundingBox.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <array>

namespace Insight
{
	namespace Graphics
	{
        class IS_GRAPHICS Plane
        {
        public:
            Plane() = default;

            Plane(const glm::vec3& normal, float d);

            // Construct from a normal vector and a point on the plane
            Plane(const glm::vec3& normal, const glm::vec3& point);

            // Construct from 3 vertices
            Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

            ~Plane() = default;

            void Normalize();
            static Plane Normalize(const Plane& plane);

            float Dot(const glm::vec3& v) const;
            static float Dot(const Plane& p, const glm::vec3& v);

            glm::vec3 normal = glm::vec3(0, 0, 0);
            float d = 0.0f; // distance from origin
        };

		class IS_GRAPHICS Frustum
		{
        public:
            Frustum() = default;
            Frustum(const glm::mat4& mView, const glm::mat4& mProjection, float screenDepth);
            ~Frustum() = default;

            bool IsVisible(const glm::vec3& center, const glm::vec3& extent, bool ignore_near_plane = false) const;
            bool IsVisible(const Graphics::BoundingBox& boundingbox) const;
            std::array<glm::vec3, 8> GetWorldPoints() const;

        private:
            Intersection CheckCube(const glm::vec3& center, const glm::vec3& extent) const;
            Intersection CheckSphere(const glm::vec3& center, float radius) const;

            Plane m_planes[6];

            glm::mat4 m_view;
            glm::mat4 m_projection;
		};
	}
}