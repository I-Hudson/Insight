#pragma once

#include "Core/Defines.h"
#include "Graphics/Defines.h"
#include "Graphics/Frustum.h"

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Insight
{
	class GFXHelperLine
	{
	public:

		GFXHelperLine(glm::vec3 start, glm::vec3 end, glm::vec4 colour);
		GFXHelperLine(glm::vec3 start, glm::vec3 direction, float distance, glm::vec4 colour);

		glm::vec3 Start;
		glm::vec3 End;
		glm::vec4 Colour;
	};

	class IS_GRAPHICS GFXHelper
	{
	public:

		void static Reset();
		void static AddLine(glm::vec3 start, glm::vec3 end, glm::vec4 colour = DefaultColour);
		void static AddLine(glm::vec3 start, glm::vec3 direction, float distance, glm::vec4 colour = DefaultColour);
		void static AddCube(glm::vec3 position, glm::vec3 extents, glm::vec4 colour = DefaultColour);
		void static AddFrustum(Graphics::Frustum frustum, glm::vec4 colour = DefaultColour);
	
		CONSTEXPR static glm::vec4 DefaultColour = glm::vec4(1, 1, 1, 1);
		static std::vector<GFXHelperLine> m_lines;
	};
}