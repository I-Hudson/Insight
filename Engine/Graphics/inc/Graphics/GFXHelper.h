#pragma once

#include "Core/Defines.h"
#include "Graphics/Defines.h"
#include "Graphics/Frustum.h"

#include <vector>
namespace Insight
{
	class GFXHelperLine
	{
	public:

		GFXHelperLine(Maths::Vector3 start, Maths::Vector3 end, Maths::Vector4 colour);
		GFXHelperLine(Maths::Vector3 start, Maths::Vector3 direction, float distance, Maths::Vector4 colour);

		Maths::Vector3 Start;
		Maths::Vector3 End;
		Maths::Vector4 Colour;
	};

	class IS_GRAPHICS GFXHelper
	{
	public:

		void static Reset();
		void static AddLine(const Maths::Vector3& start, const Maths::Vector3& end, const Maths::Vector4& colour = DefaultColour);
		void static AddLine(const Maths::Vector3& start, const Maths::Vector3& direction, float distance, const Maths::Vector4& colour = DefaultColour);
		void static AddCube(const Maths::Vector3& position, const Maths::Vector3& extents, const Maths::Vector4& colour = DefaultColour);
		void static AddFrustum(Graphics::Frustum frustum, const Maths::Vector4& colour = DefaultColour);
	
		static Maths::Vector4 DefaultColour;
		static std::vector<GFXHelperLine> m_lines;
	};
}