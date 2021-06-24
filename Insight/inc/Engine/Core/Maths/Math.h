#pragma once

#include "glm/glm.hpp"

namespace Insight::Maths
{
	bool DecomposeTranfrom(glm::mat4 const& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	glm::vec3 RadiansToDegressVector(glm::vec3 const& vec);
	glm::mat4 RotationVectorRadToMatrix(glm::vec3 const& rotation);
	glm::mat4 RotationVectorDegToMatrix(glm::vec3 const& rotation);
}