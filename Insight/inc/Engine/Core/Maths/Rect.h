#pragma once

#include "glm/vec3.hpp"

namespace Insight::Maths
{
	class Rect
	{
	public:
		Rect();
		Rect(float const& x, float const& y, float const& width, float const& height);
		Rect(Rect const& other);
		~Rect();

		bool CotainsPoint(glm::vec3 const& vec);

		float const& GetX() const { return m_x; }
		float const& GetY() const { return m_y; }
		float const& GetWidth() const { return m_width; }
		float const& GetHeight() const { return m_height; }

	private:
		float m_x;
		float m_y;
		float m_width;
		float m_height;
	};
}