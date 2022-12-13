#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Core/Memory.h"

namespace Insight
{
	namespace Input
	{
		InputDevice_KeyboardMouse::InputDevice_KeyboardMouse()
		{ }

		InputDevice_KeyboardMouse::~InputDevice_KeyboardMouse()
		{ }

		void InputDevice_KeyboardMouse::Initialise(u32 id)
		{
			m_id = id;
		}

		void InputDevice_KeyboardMouse::Shutdown()
		{
			m_id = 0;
		}

		void InputDevice_KeyboardMouse::ProcessInput(GenericInput const& input)
		{
			if (input.InputType == InputTypes::Button || input.InputType == InputTypes::MouseButton)
			{
				u32 buttonIdx = static_cast<u32>(input.Data0);
				ButtonStates buttonState = static_cast<ButtonStates>(input.Data1);
				if (buttonState == ButtonStates::Released)
				{
					if (input.InputType == InputTypes::Button)
					{
						m_buttons.ButtonReleased(buttonIdx);
					}
					else if (input.InputType == InputTypes::MouseButton)
					{
						m_mouse.ButtonReleased(buttonIdx);
					}
					
				}
				if (buttonState == ButtonStates::Pressed)
				{
					if (input.InputType == InputTypes::Button)
					{
						m_buttons.ButtonPressed(buttonIdx);
					}
					else if (input.InputType == InputTypes::MouseButton)
					{
						m_mouse.ButtonPressed(buttonIdx);
					}
				}
			}
			else if (input.InputType == InputTypes::Mouse)
			{
				double xPosition;
				double yPosition;
				Platform::MemCopy(&xPosition, &input.Data0, sizeof(xPosition));
				Platform::MemCopy(&yPosition, &input.Data1, sizeof(yPosition));
				m_mouse.MouseMoved(static_cast<float>(xPosition), static_cast<float>(yPosition));
			}
		}

		void InputDevice_KeyboardMouse::Update(float const deltaTime)
		{
			m_buttons.Update(deltaTime);
			m_mouse.Update(deltaTime);
		}

		void InputDevice_KeyboardMouse::ClearFrame()
		{
			m_buttons.ClearFrame();
			m_mouse.ClearFrame();
		}

		bool InputDevice_KeyboardMouse::WasPressed(KeyboardButtons buttonIdx) const
		{
			return m_buttons.WasPressed(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_KeyboardMouse::WasReleased(KeyboardButtons buttonIdx) const
		{
			return m_buttons.WasReleased(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_KeyboardMouse::WasHeld(KeyboardButtons buttonIdx) const
		{
			return m_buttons.WasHeld(static_cast<u32>(buttonIdx));
		}

		float Input::InputDevice_KeyboardMouse::GetMouseXPosition() const
		{
			return m_mouse.GetXPosition();
		}

		float InputDevice_KeyboardMouse::GetMouseYPosition() const
		{
			return m_mouse.GetYPosition();
		}

		bool InputDevice_KeyboardMouse::WasPressed(MouseButtons buttonIdx) const
		{
			return m_mouse.WasPressed(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_KeyboardMouse::WasReleased(MouseButtons buttonIdx) const
		{
			return m_mouse.WasPressed(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_KeyboardMouse::WasHeld(MouseButtons buttonIdx) const
		{
			return m_mouse.WasHeld(static_cast<u32>(buttonIdx));
		}
	}
}