#include "Input/GenericInput.h"

namespace Insight
{
	namespace Input
	{
		const char* InputDeviceTypeToString(InputDeviceTypes inputDeviceType)
		{
			switch (inputDeviceType)
			{
			case Insight::Input::InputDeviceTypes::KeyboardMouse:	return "KeyboardMouse";
			case Insight::Input::InputDeviceTypes::Controller:		return "Controller";
			}
			return "";
		}

		const char* InputTypeToString(InputTypes inputTypes)
		{
			switch (inputTypes)
			{
			case Insight::Input::InputTypes::Button:		return "Button";
			case Insight::Input::InputTypes::Mouse:			return "Mouse";
			case Insight::Input::InputTypes::MouseButton:	return "MouseButton";
			case Insight::Input::InputTypes::Thumbstick:	return "Thumbstick";
			case Insight::Input::InputTypes::Trigger:		return "Trigger";
			}
			return "";
		}
	}
}