#include "Input/InputMouseState.h"

namespace Insight
{
	namespace Input
	{
		InputMouseState::InputMouseState()
		{ }

		InputMouseState::~InputMouseState()
		{ }

		float InputMouseState::GetXPosition() const
		{
			return m_xPosition;
		}

		float InputMouseState::GetYPosition() const
		{
			return m_yPosition;
		}

		void InputMouseState::MouseMoved(float xPosition, float yPosition)
		{
			m_xPosition = xPosition;
			m_yPosition = yPosition;
		}
	}
}