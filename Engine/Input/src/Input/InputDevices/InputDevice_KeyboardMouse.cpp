#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Core/Memory.h"

namespace Insight
{
	namespace Input
	{
		const char* KeyboardButtonToString(KeyboardButtons button)
		{
			switch (button)
			{
			case KeyboardButtons::Key_A:             return "Key_A";
			case KeyboardButtons::Key_B:             return "Key_B";
			case KeyboardButtons::Key_C:             return "Key_C";
			case KeyboardButtons::Key_D:             return "Key_D";
			case KeyboardButtons::Key_E:             return "Key_E";
			case KeyboardButtons::Key_F:             return "Key_F";
			case KeyboardButtons::Key_G:             return "Key_G";
			case KeyboardButtons::Key_H:             return "Key_H";
			case KeyboardButtons::Key_I:             return "Key_I";
			case KeyboardButtons::Key_J:             return "Key_J";
			case KeyboardButtons::Key_K:             return "Key_K";
			case KeyboardButtons::Key_L:             return "Key_L";
			case KeyboardButtons::Key_M:             return "Key_M";
			case KeyboardButtons::Key_N:             return "Key_N";
			case KeyboardButtons::Key_O:             return "Key_O";
			case KeyboardButtons::Key_P:             return "Key_P";
			case KeyboardButtons::Key_Q:             return "Key_Q";
			case KeyboardButtons::Key_R:             return "Key_R";
			case KeyboardButtons::Key_S:             return "Key_S";
			case KeyboardButtons::Key_T:             return "Key_T";
			case KeyboardButtons::Key_U:             return "Key_U";
			case KeyboardButtons::Key_V:             return "Key_V";
			case KeyboardButtons::Key_W:             return "Key_W";
			case KeyboardButtons::Key_X:             return "Key_X";
			case KeyboardButtons::Key_Y:             return "Key_Y";
			case KeyboardButtons::Key_Z:             return "Key_Z";
			case KeyboardButtons::Key_0:             return "Key_0";
			case KeyboardButtons::Key_1:             return "Key_1";
			case KeyboardButtons::Key_2:             return "Key_2";
			case KeyboardButtons::Key_3:             return "Key_3";
			case KeyboardButtons::Key_4:             return "Key_4";
			case KeyboardButtons::Key_5:             return "Key_5";
			case KeyboardButtons::Key_6:             return "Key_6";
			case KeyboardButtons::Key_7:             return "Key_7";
			case KeyboardButtons::Key_8:             return "Key_8";
			case KeyboardButtons::Key_9:             return "Key_9";
			case KeyboardButtons::Key_Comma:         return "Key_Comma";
			case KeyboardButtons::Key_Period:	     return "Key_Period";
			case KeyboardButtons::Key_ForwardSlash:	 return "Key_ForwardSlash";
			case KeyboardButtons::Key_SemiColon:	 return "Key_SemiColon";
			case KeyboardButtons::Key_Quote:         return "Key_Quote";
			case KeyboardButtons::Key_LBracket:      return "Key_LBracket";
			case KeyboardButtons::Key_RBracket:      return "Key_RBracket";
			case KeyboardButtons::Key_BackSlash:     return "Key_BackSlash";
			case KeyboardButtons::Key_Underscore:    return "Key_Underscore";
			case KeyboardButtons::Key_Equals:        return "Key_Equals";
			case KeyboardButtons::Key_Backspace:     return "Key_Backspace";
			case KeyboardButtons::Key_Tilde:         return "Key_Tilde";
			case KeyboardButtons::Key_Tab:           return "Key_Tab";
			case KeyboardButtons::Key_CapsLock:      return "Key_CapsLock";
			case KeyboardButtons::Key_Enter:         return "Key_Enter";
			case KeyboardButtons::Key_Escape:        return "Key_Escape";
			case KeyboardButtons::Key_Space:         return "Key_Space";
			case KeyboardButtons::Key_Left:          return "Key_Left";
			case KeyboardButtons::Key_Up:            return "Key_Up";
			case KeyboardButtons::Key_Right:         return "Key_Right";
			case KeyboardButtons::Key_Down:          return "Key_Down";
			case KeyboardButtons::Key_NumLock:       return "Key_NumLock";
			case KeyboardButtons::Key_Numpad0:       return "Key_Numpad0";
			case KeyboardButtons::Key_Numpad1:       return "Key_Numpad1";
			case KeyboardButtons::Key_Numpad2:       return "Key_Numpad2";
			case KeyboardButtons::Key_Numpad3:       return "Key_Numpad3";
			case KeyboardButtons::Key_Numpad4:       return "Key_Numpad4";
			case KeyboardButtons::Key_Numpad5:       return "Key_Numpad5";
			case KeyboardButtons::Key_Numpad6:       return "Key_Numpad6";
			case KeyboardButtons::Key_Numpad7:       return "Key_Numpad7";
			case KeyboardButtons::Key_Numpad8:       return "Key_Numpad8";
			case KeyboardButtons::Key_Numpad9:       return "Key_Numpad9";
			case KeyboardButtons::Key_NumpadEnter:   return "Key_NumpadEnter";
			case KeyboardButtons::Key_NumpadMultiply:return "Key_NumpadMultiply";
			case KeyboardButtons::Key_NumpadPlus:    return "Key_NumpadPlus";
			case KeyboardButtons::Key_NumpadMinus:   return "Key_NumpadMinus";
			case KeyboardButtons::Key_NumpadPeriod:  return "Key_NumpadPeriod";
			case KeyboardButtons::Key_NumpadDivide:  return "Key_NumpadDivide";
			case KeyboardButtons::Key_F1:            return "Key_F1";
			case KeyboardButtons::Key_F2:            return "Key_F2";
			case KeyboardButtons::Key_F3:            return "Key_F3";
			case KeyboardButtons::Key_F4:            return "Key_F4";
			case KeyboardButtons::Key_F5:            return "Key_F5";
			case KeyboardButtons::Key_F6:            return "Key_F6";
			case KeyboardButtons::Key_F7:            return "Key_F7";
			case KeyboardButtons::Key_F8:            return "Key_F8";
			case KeyboardButtons::Key_F9:            return "Key_F9";
			case KeyboardButtons::Key_F10:           return "Key_F10";
			case KeyboardButtons::Key_F11:           return "Key_F11";
			case KeyboardButtons::Key_F12:           return "Key_F12";
			case KeyboardButtons::Key_F13:           return "Key_F13";
			case KeyboardButtons::Key_F14:           return "Key_F14";
			case KeyboardButtons::Key_F15:           return "Key_F15";
			case KeyboardButtons::Key_PrintScreen:   return "Key_PrintScreen";
			case KeyboardButtons::Key_ScrollLock:	 return "Key_ScrollLock";
			case KeyboardButtons::Key_Pause:         return "Key_Pause";
			case KeyboardButtons::Key_Insert:        return "Key_Insert";
			case KeyboardButtons::Key_Delete:        return "Key_Delete";
			case KeyboardButtons::Key_Home:          return "Key_Home";
			case KeyboardButtons::Key_End:           return "Key_End";
			case KeyboardButtons::Key_PageUp:        return "Key_PageUp";
			case KeyboardButtons::Key_PageDown:      return "Key_PageDown";
			case KeyboardButtons::Key_Application:   return "Key_Application";
			case KeyboardButtons::Key_Menu:			 return "Key_Menu";
			case KeyboardButtons::Key_LShift:        return "Key_LShift";
			case KeyboardButtons::Key_RShift:        return "Key_RShift";
			case KeyboardButtons::Key_LCtrl:         return "Key_LCtrl";
			case KeyboardButtons::Key_RCtrl:         return "Key_RCtrl";
			case KeyboardButtons::Key_LAlt:          return "Key_LAlt";
			case KeyboardButtons::Key_RAlt:          return "Key_RAlt";
			case KeyboardButtons::Key_LSuper:        return "Key_LSuper";
			case KeyboardButtons::Key_RSuper:        return "Key_RSuper";
			case KeyboardButtons::NumButtons:
			default:
				break;
			}
			FAIL_ASSERT();
			return "";
		}

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
			else if (input.InputType == InputTypes::MouseScroll)
			{
				double xOffset;
				double yOffset;
				Platform::MemCopy(&xOffset, &input.Data0, sizeof(xOffset));
				Platform::MemCopy(&yOffset, &input.Data1, sizeof(yOffset));
				m_mouse.MouseScrolled(static_cast<float>(xOffset), static_cast<float>(yOffset));
			}
		}

		bool InputDevice_KeyboardMouse::HasInput() const
		{
			return m_buttons.HasInput() || m_mouse.HasInput();
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

		float InputDevice_KeyboardMouse::GetMouseXScrollOffset() const
		{
			return m_mouse.GetXScollOffset();
		}

		float InputDevice_KeyboardMouse::GetMouseYScrollOffset() const
		{
			return m_mouse.GetYScollOffset();
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