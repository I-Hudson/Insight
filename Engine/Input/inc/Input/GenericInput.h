#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
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
			MouseScroll,
			MouseButton,
			Thumbstick,
			Trigger
		};
		IS_INPUT const char* InputTypeToString(InputTypes inputTypes);

		struct IS_INPUT GenericInput
		{
			u64 DevieId;
			InputDeviceTypes InputDevice;
			InputTypes InputType;
			u64 Data0 = 0;
			u64 Data1 = 0;
			u64 Data2 = 0;
		};
	}
}