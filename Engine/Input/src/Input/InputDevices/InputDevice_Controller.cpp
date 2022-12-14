#include "Input/InputDevices/InputDeivce_Controller.h"

namespace Insight
{
	namespace Input
	{
		CONSTEXPR const char* ControllerButtonsToString(ControllerButtons button)
		{
			switch (button)
			{
			case Insight::Input::ControllerButtons::A:					return "A";
			case Insight::Input::ControllerButtons::B:					return "B";
			case Insight::Input::ControllerButtons::X:					return "X";
			case Insight::Input::ControllerButtons::Y:					return "Y";
			case Insight::Input::ControllerButtons::Thumbstick_Left:	return "Thumbstick_Left";
			case Insight::Input::ControllerButtons::Thumbstick_Right:	return "Thumbstick_Right";
			case Insight::Input::ControllerButtons::DPad_Up:			return "DPad_Up";
			case Insight::Input::ControllerButtons::DPad_Right:			return "DPad_Right";
			case Insight::Input::ControllerButtons::DPad_Down:			return "DPad_Down";
			case Insight::Input::ControllerButtons::DPad_Left:			return "DPad_Left";
			case Insight::Input::ControllerButtons::Bummer_Left:		return "Bummer_Left";
			case Insight::Input::ControllerButtons::Bummber_Right:		return "Bummber_Right";
			case Insight::Input::ControllerButtons::Start:				return "Start";
			case Insight::Input::ControllerButtons::Select:				return "Select";
			case Insight::Input::ControllerButtons::Share:				return "Share";
			case Insight::Input::ControllerButtons::NumButtons:
			default:
				break;
			}
			return "";
		}

		CONSTEXPR const char* ControllerThumbstickToString(ControllerThumbsticks thumbstick)
		{
			switch (thumbstick)
			{
			case Insight::Input::ControllerThumbsticks::Left_X:	 return "Left_X";
			case Insight::Input::ControllerThumbsticks::Left_Y:  return "Left_Y";
			case Insight::Input::ControllerThumbsticks::Right_X: return "Right_X";
			case Insight::Input::ControllerThumbsticks::Right_Y: return "Right_Y";
			case Insight::Input::ControllerThumbsticks::NumThumbsticks:
			default:
				break;
			}
			return "";
		}

		CONSTEXPR const char* ControllerTriggerToString(ControllerTriggers trigger)
		{
			switch (trigger)
			{
			case Insight::Input::ControllerTriggers::Left:  return "Left";
			case Insight::Input::ControllerTriggers::Right: return "Right";
			case Insight::Input::ControllerTriggers::NumTriggers:
				break;
			default:
				break;
			}
			return "";
		}

		CONSTEXPR const char* ControllerRumbleToString(ControllerRumbles rumble)
		{
			switch (rumble)
			{
			case Insight::Input::ControllerRumbles::Left: return "Left";
			case Insight::Input::ControllerRumbles::Right: return "Right";
			case Insight::Input::ControllerRumbles::NumRumbles:
				break;
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
			else if (input.InputType == InputTypes::Thumbstick)
			{
				ControllerThumbsticks thumbstickIdx = static_cast<ControllerThumbsticks>(input.Data0);
				float scaledValue;
				i16 rawValue;
				Platform::MemCopy(&scaledValue, &input.Data1, sizeof(scaledValue));
				Platform::MemCopy(&rawValue, &input.Data2, sizeof(rawValue));

				if (thumbstickIdx == ControllerThumbsticks::Left_X)
				{
					m_leftTumbstick.SetValueX(scaledValue);
					m_leftTumbstick.SetRawValueX(rawValue);
				}
				else if (thumbstickIdx == ControllerThumbsticks::Left_Y)
				{
					m_leftTumbstick.SetValueY(scaledValue);
					m_leftTumbstick.SetRawValueY(rawValue);
				}
				else if (thumbstickIdx == ControllerThumbsticks::Right_X)
				{
					m_rightTumbstick.SetValueX(scaledValue);
					m_rightTumbstick.SetRawValueX(rawValue);
				}
				else if (thumbstickIdx == ControllerThumbsticks::Right_Y)
				{
					m_rightTumbstick.SetValueY(scaledValue);
					m_rightTumbstick.SetRawValueY(rawValue);
				}
			}
			else if (input.InputType == InputTypes::Trigger)
			{
				ControllerTriggers triggerIdx = static_cast<ControllerTriggers>(input.Data0);
				float scaledValue;
				u8 rawValue;
				Platform::MemCopy(&scaledValue, &input.Data1, sizeof(scaledValue));
				Platform::MemCopy(&rawValue, &input.Data2, sizeof(rawValue));

				if (triggerIdx == ControllerTriggers::Left)
				{
					m_leftTrigger.SetValue(scaledValue);
					m_leftTrigger.SetRawValue(rawValue);
				}
				else if (triggerIdx == ControllerTriggers::Right)
				{
					m_rightTrigger.SetValue(scaledValue);
					m_rightTrigger.SetRawValue(rawValue);
				}
			}
		}

		bool InputDevice_Controller::HasInput() const
		{
			return m_buttons.HasInput()
				|| m_leftTumbstick.HasInput()
				|| m_rightTumbstick.HasInput()
				|| m_leftTrigger.HasInput()
				|| m_rightTrigger.HasInput();
		}

		void InputDevice_Controller::Update(float const deltaTime)
		{
			m_buttons.Update(deltaTime);
			m_leftTumbstick.Update();
			m_rightTumbstick.Update();
			m_leftTrigger.Update();
			m_rightTrigger.Update();
		}

		void InputDevice_Controller::ClearFrame()
		{
			m_buttons.ClearFrame();
			m_leftTumbstick.ClearFrame();
			m_rightTumbstick.ClearFrame();
			m_leftTrigger.ClearFrame();
			m_rightTrigger.ClearFrame();
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

		float InputDevice_Controller::GetThumbstickValue(ControllerThumbsticks tumbstick) const
		{
			switch (tumbstick)
			{
			case Insight::Input::ControllerThumbsticks::Left_X: return m_leftTumbstick.GetValueX();
			case Insight::Input::ControllerThumbsticks::Left_Y: return m_leftTumbstick.GetValueY();
			case Insight::Input::ControllerThumbsticks::Right_X: return m_rightTumbstick.GetValueX();
			case Insight::Input::ControllerThumbsticks::Right_Y: return m_rightTumbstick.GetValueY();
			case Insight::Input::ControllerThumbsticks::NumThumbsticks:
			default:
				break;
			}
			return 0.0f;
		}

		i16 InputDevice_Controller::GetThumbstickRawValue(ControllerThumbsticks tumbstick) const
		{
			switch (tumbstick)
			{
			case Insight::Input::ControllerThumbsticks::Left_X: return m_leftTumbstick.GetRawValueX();
			case Insight::Input::ControllerThumbsticks::Left_Y: return m_leftTumbstick.GetRawValueY();
			case Insight::Input::ControllerThumbsticks::Right_X: return m_rightTumbstick.GetRawValueX();
			case Insight::Input::ControllerThumbsticks::Right_Y: return m_rightTumbstick.GetRawValueY();
			case Insight::Input::ControllerThumbsticks::NumThumbsticks:
			default:
				break;
			}
			return 0;
		}

		float InputDevice_Controller::GetTriggerValue(ControllerTriggers trigger) const
		{
			switch (trigger)
			{
			case Insight::Input::ControllerTriggers::Left: return m_leftTrigger.GetValue();
			case Insight::Input::ControllerTriggers::Right: return m_rightTrigger.GetValue();
			case Insight::Input::ControllerTriggers::NumTriggers:
			default:
				break;
			}
			return 0.0f;
		}

		u8 InputDevice_Controller::GetTriggerRawValue(ControllerTriggers trigger) const
		{
			switch (trigger)
			{
			case Insight::Input::ControllerTriggers::Left: return m_leftTrigger.GetRawValue();
			case Insight::Input::ControllerTriggers::Right: return m_rightTrigger.GetRawValue();
			case Insight::Input::ControllerTriggers::NumTriggers:
			default:
				break;
			}
			return 0;
		}

		void InputDevice_Controller::SetRumbleValue(ControllerRumbles rumble, float value)
		{
			m_rumbles[static_cast<u64>(rumble)].SetValue(value);
		}

		float InputDevice_Controller::GetRumbleValue(ControllerRumbles rumble) const
		{
			switch (rumble)
			{
			case Insight::Input::ControllerRumbles::Left:
			case Insight::Input::ControllerRumbles::Right:
			{
				return m_rumbles[static_cast<u64>(rumble)].GetValue();
			}
			case Insight::Input::ControllerRumbles::NumRumbles:
			default:
				break;
			}
			return 0.0f;
		}

		u16 InputDevice_Controller::GetRumbleRawValue(ControllerRumbles rumble) const
		{
			switch (rumble)
			{
			case Insight::Input::ControllerRumbles::Left:
			case Insight::Input::ControllerRumbles::Right:
			{
				return m_rumbles[static_cast<u64>(rumble)].GetRawValue();
			}
			case Insight::Input::ControllerRumbles::NumRumbles:
			default:
				break;
			}
			return 0;
		}
	}
}