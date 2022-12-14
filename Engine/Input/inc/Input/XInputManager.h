#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/TypeAlias.h"
#include "Input/GenericInput.h"

#include <array>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

namespace Insight
{
	namespace Input
	{
		class InputSystem;

		class XInputManager
		{
		public:
			XInputManager() = default;;
			~XInputManager() = default;

			void Initialise(InputSystem* inputSystem);
			void Shutdown();

			void Update();

		private:
			void ExtractDeviceInfo(u32 controllerIndex);
			void ProcessInput(u32 controllerIndex, XINPUT_STATE state);
			void ProcessVibration(u32 controllerIndex);
			void AnalogInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, int value, int deadzone, int maxValue);

		private:
			InputSystem* m_inputSystem = nullptr;
			std::array<bool, 4> m_connectedPorts;
		};
	}
}
#endif