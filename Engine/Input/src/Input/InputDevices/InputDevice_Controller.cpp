#include "Input/InputDevices/InputDeivce_Controller.h"

namespace Insight
{
	namespace Input
	{
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
	}
}