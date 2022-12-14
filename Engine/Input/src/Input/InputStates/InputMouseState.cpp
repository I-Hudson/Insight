#include "Input/InputStates/InputMouseState.h"

namespace Insight
{
	namespace Input
	{
		InputMouseState::InputMouseState()
		{ }

		InputMouseState::~InputMouseState()
		{ }

		bool InputMouseState::HasInput() const
		{
			return __super::HasInput()
				|| m_scrollXOffset != 0.0f 
				|| m_scrollYOffset != 0.0f;
		}

		void InputMouseState::ClearFrame()
		{
			__super::ClearFrame();
			m_scrollXOffset = 0.0f;
			m_scrollYOffset = 0.0f;
		}

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

		void InputMouseState::MouseScrolled(float xOffset, float yOffset)
		{
			m_scrollXOffset = xOffset;
			m_scrollYOffset = yOffset;
		}
	}
}