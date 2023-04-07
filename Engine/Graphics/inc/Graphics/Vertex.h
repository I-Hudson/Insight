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
			Vertex(glm::vec4 pos, glm::vec4 nor, glm::vec4 colour, glm::vec2 uv)
				: Position(pos), Normal(nor), Colour(colour), UV(glm::vec4(uv.x, uv.y, 0, 0))
			{ }

			glm::vec4 Position;
			glm::vec4 Normal;
			glm::vec4 Colour;
			glm::vec4 UV;

			constexpr int GetStride() { return sizeof(Vertex); }
		};
    }
}