#include "Graphics/GFXHelper.h"

namespace Insight
{
	std::vector<GFXHelperLine> GFXHelper::m_lines;

	GFXHelperLine::GFXHelperLine(glm::vec3 start, glm::vec3 end, glm::vec4 colour)
		: Start(std::move(start))
		, End(std::move(end))
		, Colour(std::move(colour))
	{ }

	GFXHelperLine::GFXHelperLine(glm::vec3 start, glm::vec3 direction, float distance, glm::vec4 colour)
		: Start(std::move(start))
		, Colour(std::move(colour))
	{
		End = start + (direction * distance);
	}

	void GFXHelper::Reset()
	{
		m_lines.clear();
	}

	void GFXHelper::AddLine(glm::vec3 start, glm::vec3 end, glm::vec4 colour)
	{
		m_lines.push_back(GFXHelperLine(start, end, colour));
	}

	void GFXHelper::AddLine(glm::vec3 start, glm::vec3 direction, float distance, glm::vec4 colour)
	{
		m_lines.push_back(GFXHelperLine(start, direction, distance, colour));
	}

	void GFXHelper::AddCube(glm::vec3 position, glm::vec3 extents, glm::vec4 colour)
	{
		glm::vec3 topCentre = position + glm::vec3(0, extents.y, 0);
		glm::vec3 topRight = topCentre + glm::vec3(extents.x, 0, 0);
		glm::vec3 topLeft = topCentre - glm::vec3(extents.x, 0, 0);

		glm::vec3 bottomCentre = position - glm::vec3(0, extents.y, 0);
		glm::vec3 bottomRight = bottomCentre + glm::vec3(extents.x, 0, 0);
		glm::vec3 bottomLeft = bottomCentre - glm::vec3(extents.x, 0, 0);

		AddLine(topLeft  + glm::vec3(0, 0, extents.z), topRight + glm::vec3(0, 0, extents.z));
		AddLine(topLeft  - glm::vec3(0, 0, extents.z), topRight - glm::vec3(0, 0, extents.z));
		AddLine(topLeft  + glm::vec3(0, 0, extents.z), topLeft  - glm::vec3(0, 0, extents.z));
		AddLine(topRight + glm::vec3(0, 0, extents.z), topRight - glm::vec3(0, 0, extents.z));

		AddLine(bottomLeft  + glm::vec3(0, 0, extents.z), bottomRight + glm::vec3(0, 0, extents.z));
		AddLine(bottomLeft  - glm::vec3(0, 0, extents.z), bottomRight - glm::vec3(0, 0, extents.z));
		AddLine(bottomLeft  + glm::vec3(0, 0, extents.z), bottomLeft  - glm::vec3(0, 0, extents.z));
		AddLine(bottomRight + glm::vec3(0, 0, extents.z), bottomRight - glm::vec3(0, 0, extents.z));

		AddLine(bottomLeft  + glm::vec3(0, 0, extents.z), topLeft  + glm::vec3(0, 0, extents.z));
		AddLine(bottomLeft  - glm::vec3(0, 0, extents.z), topLeft  - glm::vec3(0, 0, extents.z));
		AddLine(bottomRight + glm::vec3(0, 0, extents.z), topRight + glm::vec3(0, 0, extents.z));
		AddLine(bottomRight - glm::vec3(0, 0, extents.z), topRight - glm::vec3(0, 0, extents.z));
	}

	void GFXHelper::AddFrustum(Graphics::Frustum frustum, glm::vec4 colour)
	{
		std::array<glm::vec3, 8> worldPoints = frustum.GetWorldPoints();

		AddLine(worldPoints.at(0), worldPoints.at(1), colour);
		AddLine(worldPoints.at(1), worldPoints.at(2), colour);
		AddLine(worldPoints.at(2), worldPoints.at(3), colour);
		AddLine(worldPoints.at(3), worldPoints.at(0), colour);

		AddLine(worldPoints.at(4), worldPoints.at(5), colour);
		AddLine(worldPoints.at(5), worldPoints.at(6), colour);
		AddLine(worldPoints.at(6), worldPoints.at(7), colour);
		AddLine(worldPoints.at(7), worldPoints.at(4), colour);

		AddLine(worldPoints.at(0), worldPoints.at(4), colour);
		AddLine(worldPoints.at(1), worldPoints.at(5), colour);
		AddLine(worldPoints.at(2), worldPoints.at(6), colour);
		AddLine(worldPoints.at(3), worldPoints.at(7), colour);
	}
}