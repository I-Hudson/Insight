#pragma once

#include <array>
#include "glm/glm.hpp"

namespace Insight::Maths
{
	class Frustum
	{
	public:
		void Update(const glm::mat4& projection, const glm::mat4& view)
		{
			glm::mat projMatrix = projection * view;

			planes[LEFT].x = projMatrix[0].w + projMatrix[0].x;
			planes[LEFT].y = projMatrix[1].w + projMatrix[1].x;
			planes[LEFT].z = projMatrix[2].w + projMatrix[2].x;
			planes[LEFT].w = projMatrix[3].w + projMatrix[3].x;

			planes[RIGHT].x = projMatrix[0].w - projMatrix[0].x;
			planes[RIGHT].y = projMatrix[1].w - projMatrix[1].x;
			planes[RIGHT].z = projMatrix[2].w - projMatrix[2].x;
			planes[RIGHT].w = projMatrix[3].w - projMatrix[3].x;

			planes[TOP].x = projMatrix[0].w - projMatrix[0].y;
			planes[TOP].y = projMatrix[1].w - projMatrix[1].y;
			planes[TOP].z = projMatrix[2].w - projMatrix[2].y;
			planes[TOP].w = projMatrix[3].w - projMatrix[3].y;

			planes[BOTTOM].x = projMatrix[0].w + projMatrix[0].y;
			planes[BOTTOM].y = projMatrix[1].w + projMatrix[1].y;
			planes[BOTTOM].z = projMatrix[2].w + projMatrix[2].y;
			planes[BOTTOM].w = projMatrix[3].w + projMatrix[3].y;

			planes[BACK].x = projMatrix[0].w + projMatrix[0].z;
			planes[BACK].y = projMatrix[1].w + projMatrix[1].z;
			planes[BACK].z = projMatrix[2].w + projMatrix[2].z;
			planes[BACK].w = projMatrix[3].w + projMatrix[3].z;

			planes[FRONT].x = projMatrix[0].w - projMatrix[0].z;
			planes[FRONT].y = projMatrix[1].w - projMatrix[1].z;
			planes[FRONT].z = projMatrix[2].w - projMatrix[2].z;
			planes[FRONT].w = projMatrix[3].w - projMatrix[3].z;

			for (auto& plane : planes)
			{
				float length = sqrtf(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
				plane /= length;
			}
		}

		bool CheckSphere(const glm::vec3& pos, float radius)
		{
			for (auto& plane : planes)
			{
				if ((plane.x * pos.x) + (plane.y * pos.y) + (plane.z * pos.z) + plane.w <= -radius)
				{
					return false;
				}
			}
			return true;
		}

	private:
		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
		std::array<glm::vec4, 6> planes;
	};
}