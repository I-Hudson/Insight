#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Core/UUID.h"

#include <../vendor/glm/glm/glm.hpp>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec4 Colour;
	glm::vec2 UV1;
};

namespace Insight
{
	namespace Render
	{
		class VulkanRenderer;
		class Swapchain;

		class VertexBuffer : public Insight::UUID
		{
		public:
			VertexBuffer() : Insight::UUID() { }
			virtual ~VertexBuffer() { }

			static VertexBuffer* Create(const std::vector<Vertex>& vertices);
		};

		class IndexBuffer : public Insight::UUID
		{
		public:
			IndexBuffer() : Insight::UUID() { }
			virtual ~IndexBuffer() { }

			static IndexBuffer* Create(const std::vector<unsigned int>& indices);
		};
	}
}