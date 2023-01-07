#include "Input/InputStates/InputMouseState.h"

namespace Insight
{
	namespace Input
	{
		const char* MouseButtonToString(MouseButtons button)
		{
			switch (button)
			{
			case Insight::Input::MouseButtons::Left:	return "Left";
			case Insight::Input::MouseButtons::Right:	return "Right";
			case Insight::Input::MouseButtons::Middle:	return "Middle";
			case Insight::Input::MouseButtons::Side0:	return "Side0";
			case Insight::Input::MouseButtons::Side1:	return "Side1";
			case Insight::Input::MouseButtons::NumButtons:
			default:
				break;
			}
			return "";
		}

		InputMouseState::InputMouseState()
		{ }

		InputMouseState::~InputMouseState()
		{ }

		bool InputMouseState::HasInput() const
		{
			return __super::HasInput()
				|| m_scrollXOffset != 0.0f 
				|| m_scrollYOffset != 0.0f
				|| m_mouseMoved;
		}

		void InputMouseState::ClearFrame()
		{
			__super::ClearFrame();
			m_scrollXOffset = 0.0f;
			m_scrollYOffset = 0.0f;
			m_mouseMoved = false;
		}

		float InputMouseState::GetXPosition() const
		{
			return m_xPosition;
		}

		float InputMouseState::GetYPosition() const
		{
			return m_yPosition;
		}

		float InputMouseState::GetXScollOffset() const
		{
			return m_scrollXOffset;
		}

		float InputMouseState::GetYScollOffset() const
		{
			return m_scrollYOffset;
		}

		void InputMouseState::MouseMoved(float xPosition, float yPosition)
		{
			m_xPosition = xPosition;
			m_yPosition = yPosition;
			m_mouseMoved = true;
		}

		void InputMouseState::MouseScrolled(float xOffset, float yOffset)
		{
			m_scrollXOffset = xOffset;
			m_scrollYOffset = yOffset;
		}
	}
}