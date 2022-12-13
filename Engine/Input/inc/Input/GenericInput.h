#pragma once

#include "Core/TypeAlias.h"
#include "Input/Defines.h"

namespace Insight
{
	namespace Input
	{
		enum class InputDeviceTypes
		{
			KeyboardMouse,
			Controller
		};
		IS_INPUT const char* InputDeviceTypeToString(InputDeviceTypes inputDeviceType);

		enum class InputTypes
		{
			Button,
			Mouse,
			MouseButton,
			Joystick
		};
		IS_INPUT const char* InputTypeToString(InputTypes inputTypes);

		struct IS_INPUT GenericInput
		{
			u64 DevieId;
			InputDeviceTypes InputDevice;
			InputTypes InputType;
			u64 Data0;
			u64 Data1;
			u64 Data2;
		};
	}
}