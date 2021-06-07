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

			planes[LEFT].x = projMatrix[3].x + projMatrix[0].x;
			planes[LEFT].y = projMatrix[3].y + projMatrix[0].y;
			planes[LEFT].z = projMatrix[3].z + projMatrix[0].x;
			planes[LEFT].w = projMatrix[3].w + projMatrix[0].w;

			planes[RIGHT].x = projMatrix[3].x - projMatrix[0].x;
			planes[RIGHT].y = projMatrix[3].y - projMatrix[0].y;
			planes[RIGHT].z = projMatrix[3].z - projMatrix[0].z;
			planes[RIGHT].w = projMatrix[3].w - projMatrix[0].w;

			planes[TOP].x = projMatrix[3].x - projMatrix[1].x;
			planes[TOP].y = projMatrix[3].y - projMatrix[1].y;
			planes[TOP].z = projMatrix[3].z - projMatrix[1].z;
			planes[TOP].w = projMatrix[3].w - projMatrix[1].w;

			planes[BOTTOM].x = projMatrix[3].x + projMatrix[1].x;
			planes[BOTTOM].y = projMatrix[3].y + projMatrix[1].y;
			planes[BOTTOM].z = projMatrix[3].z + projMatrix[1].z;
			planes[BOTTOM].w = projMatrix[3].w + projMatrix[1].w;

			planes[FRONT].x = projMatrix[3].x + projMatrix[3].x;;
			planes[FRONT].y = projMatrix[3].y + projMatrix[3].y;;
			planes[FRONT].z = projMatrix[3].z + projMatrix[3].z;;
			planes[FRONT].w = projMatrix[3].w + projMatrix[3].w;;

			planes[BACK].x = projMatrix[3].x - projMatrix[3].x;
			planes[BACK].y = projMatrix[3].y - projMatrix[3].y;
			planes[BACK].z = projMatrix[3].z - projMatrix[3].z;
			planes[BACK].w = projMatrix[3].w - projMatrix[3].w;

			for (auto& plane : planes)
			{
				float length = sqrtf(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
				plane /= length;
			}
		}

		bool CheckSphere(const glm::vec3& pos, float radius)
		{
			bool b = true;
			for (auto& plane : planes)
			{
				float dist = glm::distance(glm::vec3(plane.xyz), pos);
				if (dist <= -radius)
				{
					return false;
				}
			}
			return true;
		}

	private:
		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, FRONT = 4, BACK = 5 };
		std::array<glm::vec4, 6> planes;
	};
}