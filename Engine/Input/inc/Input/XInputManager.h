#pragma once

#if defined(IS_PLATFORM_WINDOWS) 

#include "Core/TypeAlias.h"
#include "Input/GenericInput.h"

#include <vector>

#include <array>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

namespace Insight
{
	namespace Input
	{
		class InputSystem;

		class XInputManager : public InputManager
		{
		public:
			XInputManager();
			virtual ~XInputManager() override;

			virtual void Initialise(InputSystem* inputSystem) override;
			virtual void Shutdown() override;
			virtual void Update(float const deltaTime) override;

			static void ThumbstickInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 idX, u32 idY, float valueX, float valueY, float deadzone, float maxValue);
			static void TriggerInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, float value, float deadzone, float maxValue);

		private:
			void ExtractDeviceInfo(u32 controllerIndex);
			void ProcessInput(u32 controllerIndex, XINPUT_STATE state);
			void ProcessVibration(u32 controllerIndex);
			static void AnalogInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, float value, float deadzone, float maxValue);

		private:
			InputSystem* m_inputSystem = nullptr;
			std::array<bool, 4> m_connectedPorts = { false };
		};
	}
}
#endif // #if defined(IS_PLATFORM_WINDOWS) 