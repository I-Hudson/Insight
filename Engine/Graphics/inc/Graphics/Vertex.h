#pragma once

#include "Graphics/Defines.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Insight
{
    namespace Graphics
    {
		struct IS_GRAPHICS Vertex
		{
			Vertex()
			{ }
			Vertex(glm::vec3 pos, glm::vec3 nor, glm::vec3 colour, glm::vec2 uv)
				: Position(pos), Normal(nor), Colour(colour), UV(uv)
			{ }

			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Colour;
			glm::vec2 UV;

			constexpr int GetStride() { return sizeof(Vertex); }
		};
    }
}