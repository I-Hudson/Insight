#include "Input/InputDevices/InputDeivce_Controller.h"

namespace Insight
{
	namespace Input
	{
		CONSTEXPR const char* ControllerButtonsToString(ControllerButtons button)
		{
			switch (button)
			{
			case Insight::Input::ControllerButtons::A:              return "A";
			case Insight::Input::ControllerButtons::B:              return "B";
			case Insight::Input::ControllerButtons::X:              return "X";
			case Insight::Input::ControllerButtons::Y:              return "Y";
			case Insight::Input::ControllerButtons::Joystick_Left:  return "Joystick_Left";
			case Insight::Input::ControllerButtons::Joystick_Right: return "Joystick_Right";
			case Insight::Input::ControllerButtons::DPad_Up:        return "DPad_Up";
			case Insight::Input::ControllerButtons::DPad_Right:     return "DPad_Right";
			case Insight::Input::ControllerButtons::DPad_Down:      return "DPad_Down";
			case Insight::Input::ControllerButtons::DPad_Left:      return "DPad_Left";
			case Insight::Input::ControllerButtons::Bummer_Left:    return "Bummer_Left";
			case Insight::Input::ControllerButtons::Bummber_Right:  return "Bummber_Right";
			case Insight::Input::ControllerButtons::Start:          return "Start";
			case Insight::Input::ControllerButtons::Select:         return "Select";
			case Insight::Input::ControllerButtons::Share:          return "Share";
			case Insight::Input::ControllerButtons::NumButtons:
			default:
				break;
			}
			return "";
		}

		void InputDevice_Controller::Initialise(u32 id)
		{
			m_id = id;
		}

		void InputDevice_Controller::Shutdown()
		{
		}

		void InputDevice_Controller::ProcessInput(GenericInput const& input)
		{
			if (input.InputType == InputTypes::Button)
			{
				u32 buttonIdx = static_cast<u32>(input.Data0);
				ButtonStates buttonState = static_cast<ButtonStates>(input.Data1);
				if (buttonState == ButtonStates::Released)
				{
					m_buttons.ButtonReleased(buttonIdx);
				}
				if (buttonState == ButtonStates::Pressed)
				{
					m_buttons.ButtonPressed(buttonIdx);
				}
			}
		}

		void InputDevice_Controller::Update(float const deltaTime)
		{
			m_buttons.Update(deltaTime);
		}

		void InputDevice_Controller::ClearFrame()
		{
			m_buttons.ClearFrame();
		}

		bool InputDevice_Controller::WasPressed(ControllerButtons buttonIdx) const
		{
			return m_buttons.WasPressed(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_Controller::WasReleased(ControllerButtons buttonIdx) const
		{
			return m_buttons.WasReleased(static_cast<u32>(buttonIdx));
		}

		bool InputDevice_Controller::WasHeld(ControllerButtons buttonIdx) const
		{
			return m_buttons.WasHeld(static_cast<u32>(buttonIdx));
		}
	}
}