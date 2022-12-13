#include "Input/XInputManager.h"

#ifdef IS_PLATFORM_WINDOWS

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDeivce_Controller.h"
#include "Input/InputButtonState.h"

#include <vector>
#include <unordered_map>

namespace Insight
{
	namespace Input
	{
		const std::unordered_map<int, ControllerButtons> XINPUT_BUTTON_TO_INTERNAL =
		{
			{ 0x0001, ControllerButtons::DPad_Up },
			{ 0x0002, ControllerButtons::DPad_Down },
			{ 0x0004, ControllerButtons::DPad_Left },
			{ 0x0008, ControllerButtons::DPad_Right },

			{ 0x0010, ControllerButtons::Start },
			{ 0x0020, ControllerButtons::Select },
			{ 0x0040, ControllerButtons::Joystick_Left },
			{ 0x0080, ControllerButtons::Joystick_Right },
			{ 0x0100, ControllerButtons::Bummer_Left },
			{ 0x0200, ControllerButtons::Bummber_Right },

			{ 0x1000, ControllerButtons::A },
			{ 0x2000, ControllerButtons::B },
			{ 0x4000, ControllerButtons::X },
			{ 0x8000, ControllerButtons::Y },
		};

		void XInputManager::Initialise(InputSystem* inputSystem)
		{
			m_inputSystem = inputSystem;
		}

		void XInputManager::Shutdown()
		{
		}

		void XInputManager::Update()
		{
			DWORD dwResult;
			for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				XINPUT_STATE state;
				Platform::MemSet(&state, 0, sizeof(XINPUT_STATE));

				// Simply get the state of the controller from XInput.
				dwResult = XInputGetState(i, &state);

				if (dwResult == ERROR_SUCCESS)
				{
					// Controller is connected
					if (!m_connectedPorts.at(i))
					{
						// New connection.
						m_connectedPorts.at(i) = true;
						m_inputSystem->AddInputDevice(InputDeviceTypes::Controller, i);
					}
					ProcessInput(i, state);
				}
				else
				{
					// Controller is not connected
					if (m_connectedPorts.at(i))
					{
						// Lost connection.
						m_connectedPorts.at(i) = false;
						m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, i);
					}
				}
			}
		}

		void XInputManager::ProcessInput(u32 controllerIndex, XINPUT_STATE state)
		{
			std::vector<Input::GenericInput> inputs;

			for (auto const& iter : XINPUT_BUTTON_TO_INTERNAL)
			{
				int mask = iter.first;
				ControllerButtons controllerButton = iter.second;
				int buttonState = state.Gamepad.wButtons & mask;
				inputs.push_back(
					Input::GenericInput
					{
						static_cast<u64>(controllerIndex),
						InputDeviceTypes::Controller,
						InputTypes::Button,
						static_cast<u64>(controllerButton),
						static_cast<u64>(buttonState == 0 ? ButtonStates::Released : ButtonStates::Pressed),
						static_cast<u64>(0)
					});
			}

			m_inputSystem->UpdateInputs(inputs);
		}
	}
}
#endif