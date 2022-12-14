#include "Input/XInputManager.h"

#ifdef IS_PLATFORM_WINDOWS

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDeivce_Controller.h"
#include "Input/InputStates/InputButtonState.h"

#include "Core/Logger.h"

#include <unordered_map>

struct XINPUT_CAPABILITIES_EX
{
	XINPUT_CAPABILITIES Capabilities;
	WORD vendorId;
	WORD productId;
	WORD revisionId;
	DWORD a4; //unknown
};

typedef DWORD(_stdcall* _XInputGetCapabilitiesEx)(DWORD a1, DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES_EX* pCapabilities);
_XInputGetCapabilitiesEx XInputGetCapabilitiesEx;

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
			{ 0x0040, ControllerButtons::Thumbstick_Left },
			{ 0x0080, ControllerButtons::Thumbstick_Right },
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
						IInputDevice* device = m_inputSystem->AddInputDevice(InputDeviceTypes::Controller, i);
						InputDevice_Controller* deviceController = static_cast<InputDevice_Controller*>(device);
						deviceController->m_subType = ControllerSubTypes::Xbox360;
						ExtractDeviceInfo(i);
					}
					ProcessInput(i, state);
					ProcessVibration(i);
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

		void XInputManager::ExtractDeviceInfo(u32 controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}

			HMODULE moduleHandle = LoadLibrary(TEXT("XInput1_4.dll"));
			if (moduleHandle)
			{
				XInputGetCapabilitiesEx = (_XInputGetCapabilitiesEx)GetProcAddress(moduleHandle, (char*)108);
				if (XInputGetCapabilitiesEx)
				{
					XINPUT_CAPABILITIES_EX capsEx;
					if (XInputGetCapabilitiesEx(1, controllerIndex, 0, &capsEx) == ERROR_SUCCESS)
					{
						Platform::MemCopy(&device->m_deviceInfo.VendorId, &capsEx.vendorId, sizeof(capsEx.vendorId));
						Platform::MemCopy(&device->m_deviceInfo.ProductId, &capsEx.productId, sizeof(capsEx.productId));
						Platform::MemCopy(&device->m_deviceInfo.RevisionId, &capsEx.revisionId, sizeof(capsEx.revisionId));
					}
				}
				FreeLibrary(moduleHandle);
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
			AnalogInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Left_X), static_cast<int>(state.Gamepad.sThumbLX), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, SHRT_MAX);
			AnalogInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Left_Y), static_cast<int>(state.Gamepad.sThumbLY), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, SHRT_MAX);
			AnalogInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Right_X), static_cast<int>(state.Gamepad.sThumbRX), XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, SHRT_MAX);
			AnalogInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Right_Y), static_cast<int>(state.Gamepad.sThumbRY), XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, SHRT_MAX);

			AnalogInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Left), static_cast<int>(state.Gamepad.bLeftTrigger), XINPUT_GAMEPAD_TRIGGER_THRESHOLD, UCHAR_MAX);
			AnalogInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Right), static_cast<int>(state.Gamepad.bRightTrigger), XINPUT_GAMEPAD_TRIGGER_THRESHOLD, UCHAR_MAX);

			m_inputSystem->UpdateInputs(inputs);
		}

		void XInputManager::ProcessVibration(u32 controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}
			XINPUT_VIBRATION state = {};
			state.wLeftMotorSpeed = static_cast<WORD>(device->GetRumbleValue(ControllerRumbles::Left) * _UI16_MAX);
			state.wRightMotorSpeed = static_cast<WORD>(device->GetRumbleValue(ControllerRumbles::Right) * _UI16_MAX);
			DWORD dwResult = XInputSetState(controllerIndex, &state);
			if (dwResult != ERROR_SUCCESS)
			{
				IS_CORE_ERROR("[XInputManager::ProcessVibration] XInputSetState failed.");
			}
		}

		void XInputManager::AnalogInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, int rawValue, int deadzone, int maxValue)
		{
			if (std::abs(rawValue) < deadzone)
			{
				return;
			}

			int sign = rawValue >= 0 ? 1 : - 1;
			rawValue = std::abs(rawValue);

			// Remap the value from deadzone -> maxValue, to 0 -> maxValue.
			const float rawValueDeadZone = static_cast<float>(rawValue - deadzone);
			const float maxValueDeadZone = static_cast<float>(maxValue - deadzone);

			float scaledValue = rawValueDeadZone / maxValueDeadZone;
			scaledValue = std::max(-1.0f, std::min(1.0f, scaledValue));

			scaledValue *= sign;
			rawValue *= sign;

			u64 scaledAnalogValue = 0;
			u64 rawAnalogValue = 0;
			Platform::MemCopy(&scaledAnalogValue, &scaledValue, sizeof(scaledValue));
			Platform::MemCopy(&rawAnalogValue, &rawValue, sizeof(rawValue));

			inputs.push_back(Input::GenericInput
				{
					static_cast<u64>(controllerIndex),
					InputDeviceTypes::Controller,
					inputType,
					static_cast<u64>(id),
					scaledAnalogValue,
					rawAnalogValue
				});
		}
	}
}
#endif