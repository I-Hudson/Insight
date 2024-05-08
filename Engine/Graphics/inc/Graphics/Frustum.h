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

#include "Maths/Matrix4.h"

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
            Plane(const Maths::Vector3& normal);
            Plane(const Maths::Vector3& normal, float d);
            ~Plane() = default;

            Plane Normalize();
            Plane Normalized() const { return Plane(normal).Normalize(); }

            float Dot(const Maths::Vector3& v) const;
            static float Dot(const Plane& p, const Maths::Vector3& v);

            Maths::Vector3 normal;
            float d = 0.0f; // distance from origin
        };

		class IS_GRAPHICS Frustum
		{
        public:
            Frustum() = default;
            Frustum(const glm::mat4& mView, const glm::mat4& mProjection, float screenDepth);
            Frustum(const Maths::Matrix4& mView, const Maths::Matrix4& mProjection, float screenDepth);
            ~Frustum() = default;

            bool IsVisible(const Maths::Vector3& center, const Maths::Vector3& extent, bool ignore_near_plane = false) const;
            bool IsVisible(const Graphics::BoundingBox& boundingbox) const;
            std::array<glm::vec3, 8> GetWorldPoints() const;

        private:
            Intersection CheckSphere(const Maths::Vector3& center, float radius) const;
            Intersection CheckCube(const glm::vec3& center, const glm::vec3& extent) const;

            Plane m_planes[6];

            Maths::Matrix4 m_projectionMatrix;
            glm::mat4 m_view;
            glm::mat4 m_projection;
		};
	}
}