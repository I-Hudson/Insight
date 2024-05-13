#include "Graphics/GFXHelper.h"

namespace Insight
{
	Maths::Vector4 GFXHelper::DefaultColour = Maths::Vector4(1, 1, 1, 1);
	std::vector<GFXHelperLine> GFXHelper::m_lines;

	GFXHelperLine::GFXHelperLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector4 colour)
		: Start(std::move(start))
		, End(std::move(end))
		, Colour(std::move(colour))
	{ }

	GFXHelperLine::GFXHelperLine(Maths::Vector3 start, Maths::Vector3 direction, float distance, Maths::Vector4 colour)
		: Start(std::move(start))
		, Colour(std::move(colour))
	{
		End = start + (direction * distance);
	}

	void GFXHelper::Reset()
	{
		m_lines.clear();
	}

	void GFXHelper::AddLine(const Maths::Vector3& start, const Maths::Vector3& end, const Maths::Vector4& colour)
	{
		m_lines.push_back(GFXHelperLine(start, end, colour));
	}

	void GFXHelper::AddLine(const Maths::Vector3& start, const Maths::Vector3& direction, float distance, const Maths::Vector4& colour)
	{
		m_lines.push_back(GFXHelperLine(start, direction, distance, colour));
	}

	void GFXHelper::AddCube(const Maths::Vector3& position, const Maths::Vector3& extents, const Maths::Vector4& colour)
	{
		const Maths::Vector3 topCentre = position + Maths::Vector3(0, extents.y, 0);
		const Maths::Vector3 topRight = topCentre + Maths::Vector3(extents.x, 0, 0);
		const Maths::Vector3 topLeft = topCentre - Maths::Vector3(extents.x, 0, 0);

		const Maths::Vector3 bottomCentre = position - Maths::Vector3(0, extents.y, 0);
		const Maths::Vector3 bottomRight = bottomCentre + Maths::Vector3(extents.x, 0, 0);
		const Maths::Vector3 bottomLeft = bottomCentre - Maths::Vector3(extents.x, 0, 0);

		AddLine(topLeft  + Maths::Vector3(0, 0, extents.z), topRight + Maths::Vector3(0, 0, extents.z));
		AddLine(topLeft  - Maths::Vector3(0, 0, extents.z), topRight - Maths::Vector3(0, 0, extents.z));
		AddLine(topLeft  + Maths::Vector3(0, 0, extents.z), topLeft  - Maths::Vector3(0, 0, extents.z));
		AddLine(topRight + Maths::Vector3(0, 0, extents.z), topRight - Maths::Vector3(0, 0, extents.z));

		AddLine(bottomLeft  + Maths::Vector3(0, 0, extents.z), bottomRight + Maths::Vector3(0, 0, extents.z));
		AddLine(bottomLeft  - Maths::Vector3(0, 0, extents.z), bottomRight - Maths::Vector3(0, 0, extents.z));
		AddLine(bottomLeft  + Maths::Vector3(0, 0, extents.z), bottomLeft  - Maths::Vector3(0, 0, extents.z));
		AddLine(bottomRight + Maths::Vector3(0, 0, extents.z), bottomRight - Maths::Vector3(0, 0, extents.z));

		AddLine(bottomLeft  + Maths::Vector3(0, 0, extents.z), topLeft  + Maths::Vector3(0, 0, extents.z));
		AddLine(bottomLeft  - Maths::Vector3(0, 0, extents.z), topLeft  - Maths::Vector3(0, 0, extents.z));
		AddLine(bottomRight + Maths::Vector3(0, 0, extents.z), topRight + Maths::Vector3(0, 0, extents.z));
		AddLine(bottomRight - Maths::Vector3(0, 0, extents.z), topRight - Maths::Vector3(0, 0, extents.z));
	}

	void GFXHelper::AddFrustum(Graphics::Frustum frustum, const Maths::Vector4& colour)
	{
		std::array<Maths::Vector3, 8> worldPoints = frustum.GetWorldPoints();

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