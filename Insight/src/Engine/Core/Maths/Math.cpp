#include "ispch.h"
#include "Engine/Core/Maths/Math.h"

#include "glm/gtx/matrix_decompose.hpp"

namespace Insight::Maths
{
	bool DecomposeTranfrom(glm::mat4 const& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		//From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat localMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(localMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return true;

		// First, isolate perspective. 
		if (epsilonNotEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition.
			localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
			localMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(localMatrix[3]);
		localMatrix[3] = vec4(0, 0, 0, localMatrix[3].w);

		vec3 row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
		{
			for (length_t j = 0; j < 3; ++j)
			{
				row[i][j] = localMatrix[i][j];
			}
		}

		// Compute X scale factor and normalize fire row.
		scale.x = length(row[0]);
		row[0] = detail::scale(row[0], static_cast<T>(1));
		scale.y = length(row[1]);
		row[1] = detail::scale(row[1], static_cast<T>(1));
		scale.z = length(row[2]);
		row[2] = detail::scale(row[2], static_cast<T>(1));

		// At this point the matrix (in rows) is orthonormal.
		// Check for a coordinate system flip.If the determinant 
		// is -1 then negate the matrix and the scaling factors.

#if 0
		Pdum3 = cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; ++i)
			{
				scale[i] *= static_cast<T>(-1);
				row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-row[0][2]);
		if (cos(rotation.y) != 0)
		{
			rotation.x = atan2(row[1][2], row[2][2]);
			rotation.z = atan2(row[0][1], row[0][0]);
		}
		else
		{
			rotation.x = atan2(-row[2][0], row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

	glm::vec3 RadiansToDegressVector(glm::vec3 const& vec)
	{
		return glm::vec3(glm::degrees(vec.x), glm::degrees(vec.y), glm::degrees(vec.z));
	}

	glm::mat4 RotationVectorRadToMatrix(glm::vec3 const& rotation)
	{
		glm::mat4 matrix(1);
		matrix = glm::rotate(matrix, rotation.x, glm::vec3(1, 0, 0));
		matrix = glm::rotate(matrix, rotation.y, glm::vec3(0, 1, 0));
		matrix = glm::rotate(matrix, rotation.z, glm::vec3(0, 0, 1));
		return matrix;
	}

	glm::mat4 RotationVectorDegToMatrix(glm::vec3 const& rotation)
	{
		glm::mat4 matrix(1);
		matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		return matrix;
	}
}