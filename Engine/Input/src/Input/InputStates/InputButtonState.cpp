#include "Input/InputStates/InputButtonState.h"

namespace Insight
{
	namespace Input
	{
		InputButton::InputButton()
		{ }

		InputButton::~InputButton()
		{ }

		ButtonStates InputButton::GetState() const
		{
			return m_state;
		}

		bool InputButton::WasPressed() const
		{
			return m_state == ButtonStates::Pressed;
		}

		bool InputButton::WasReleased() const
		{
			return m_state == ButtonStates::Released;
		}

		bool InputButton::WasHeld() const
		{
			return (m_state == ButtonStates::Held && m_heldTime > 0.0f)
				|| m_state == ButtonStates::Pressed;
		}

		float InputButton::GetHeldTIme() const
		{
			return m_heldTime;
		}

		void InputButton::Update(float const deltaTime)
		{
			if (m_buttonUpdate == ButtonUpdate::UpdatedThisFrame)
			{
				// Button has been updated this frame. Leave internal update to next frame.
				m_buttonUpdate = ButtonUpdate::UpdateProgression;
			}
			else
			{
				if (m_state == ButtonStates::Pressed)
				{
					m_state = ButtonStates::Held;
				}
				else if (m_state == ButtonStates::Released)
				{
					m_state = ButtonStates::None;
				}
			}

			if (m_state == ButtonStates::Held)
			{
				m_heldTime += deltaTime;
			}
		}

		bool InputButton::HasInput() const
		{
			return WasPressed() || WasReleased() || WasHeld();
		}

		void InputButton::ClearFrame()
		{
		}

		void InputButton::SetState(ButtonStates newState)
		{	
			if ((m_state == ButtonStates::Pressed || m_state == ButtonStates::Held)
				&& newState == ButtonStates::Released)
			{
				m_state = newState;
				m_buttonUpdate = ButtonUpdate::UpdatedThisFrame;
				m_heldTime = 0.0f;
			}
			else  if ((m_state == ButtonStates::Released || m_state == ButtonStates::None)
				&& newState == ButtonStates::Pressed)
			{
				m_state = newState;
				m_buttonUpdate = ButtonUpdate::UpdatedThisFrame;
				m_heldTime = 0.0f;
			}
		}
	}
}