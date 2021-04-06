#include "ispch.h"
#include "Engine/Core/Maths/Rect.h"

namespace Insight::Maths
{
	Rect::Rect()
		: m_x(0)
		, m_y(0)
		, m_width(0)
		, m_height(0)
	{ }

	Rect::Rect(float const& x, float const& y, float const& width, float const& height)
		: m_x(x)
		, m_y(y)
		, m_width(width)
		, m_height(height)
	{ }

	Rect::Rect(Rect const& other)
	{
		m_x = other.m_x;
		m_y = other.m_y;
		m_width = other.m_width;
		m_height = other.m_height;
	}

	Rect::~Rect()
	{
	}

	bool Rect::CotainsPoint(glm::vec3 const& vec)
	{
		return	m_x < vec.x && 
				m_x + m_width > vec.x &&
				m_y < vec.y &&
				m_y + m_height > vec.y;
	}
}