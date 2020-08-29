#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"

#include <../vendor/glm/glm/glm.hpp>

struct Vertex
{
	glm::vec4 Position;
	glm::vec4 Colour;
	glm::vec4 Normal;
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