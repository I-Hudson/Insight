#include "Input/XInputManager.h"
#include "Platforms/Platform.h"

#if defined(IS_PLATFORM_WINDOWS) 

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
			for (size_t i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if (m_connectedPorts.at(i))
				{
					m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, static_cast<u32>(i));
					m_connectedPorts.at(i) = false;
				}
			}
		}

		void XInputManager::Update(float const deltaTime)
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

		void XInputManager::ThumbstickInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 idX, u32 idY, float valueX, float valueY, float deadzone, float maxValue)
		{
			// choose a deadzone -- readings inside this radius are ignored.
			const float deadzoneRadius = deadzone;
			const float deadzoneSquared = deadzoneRadius * deadzoneRadius;

			// Pythagorean theorem -- for a right triangle, hypotenuse^2 = (opposite side)^2 + (adjacent side)^2
			auto adjacentSquared = valueX * valueX;
			auto oppositeSquared = valueY * valueY;

			// accept and process input if true; otherwise, reject and ignore it.
			if ((oppositeSquared + adjacentSquared) >= deadzoneSquared)
			{
				AnalogInput(inputs, controllerIndex, inputType, idX, valueX, deadzone, maxValue);
				AnalogInput(inputs, controllerIndex, inputType, idY, valueY, deadzone, maxValue);
			}
		}

		void XInputManager::TriggerInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, float value, float deadzone, float maxValue)
		{
			if (value >= deadzone)
			{
				AnalogInput(inputs, controllerIndex, inputType, id, value, deadzone, maxValue);
			}
		}

		void XInputManager::ExtractDeviceInfo(u32 controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
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

			device->m_vendor = VendorIdToControllerVendor(device->m_deviceInfo.VendorId);
			device->m_subType = ProductIdToControllerSubType(device->m_deviceInfo.ProductId);
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
			ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Left_X), static_cast<u32>(ControllerThumbsticks::Left_Y),
				static_cast<float>(state.Gamepad.sThumbLX), static_cast<float>(state.Gamepad.sThumbLY), static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE), static_cast<float>(SHRT_MAX));
			ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Right_X), static_cast<u32>(ControllerThumbsticks::Right_Y),
				static_cast<float>(state.Gamepad.sThumbRX), static_cast<float>(state.Gamepad.sThumbRY), static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE), static_cast<float>(SHRT_MAX));
			TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Left), static_cast<float>(state.Gamepad.bLeftTrigger), 
				static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD), static_cast<float>(UCHAR_MAX));
			TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Right), static_cast<float>(state.Gamepad.bRightTrigger),
				static_cast<float>(XINPUT_GAMEPAD_TRIGGER_THRESHOLD), static_cast<float>(UCHAR_MAX));

			m_inputSystem->UpdateInputs(inputs);
		}

		void XInputManager::ProcessVibration(u32 controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}
			XINPUT_VIBRATION state = {};
			state.wLeftMotorSpeed = static_cast<WORD>(device->GetRumbleValue(ControllerRumbles::Left) * _UI16_MAX);
			state.wRightMotorSpeed = static_cast<WORD>(device->GetRumbleValue(ControllerRumbles::Right) * _UI16_MAX);
			DWORD dwResult = XInputSetState(controllerIndex, &state);
			if (dwResult != ERROR_SUCCESS)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] XInputSetState failed.");
			}
		}

		void XInputManager::AnalogInput(std::vector<GenericInput>& inputs, u32 controllerIndex, InputTypes inputType, u32 id, float value, float deadzone, float maxValue)
		{
			int sign = value >= 0 ? 1 : -1;
			value = std::abs(value);

#ifdef XINPUT_RESCALE_ANALOG_INPUT
			// Remap the value from deadzone -> maxValue, to 0 -> maxValue.
			const float rawValueDeadZone = value - deadzone;
			const float maxValueDeadZone = maxValue - deadzone;

			float scaledValue = rawValueDeadZone / maxValueDeadZone;
			scaledValue = std::max(-1.0f, std::min(1.0f, scaledValue));

			scaledValue *= sign;
			value *= sign;
#else 
			if (maxValue > 1.0f)
			{
				value = value / maxValue;
			}

			value *= sign;
#endif
			u64 valueU64 = 0;
			Platform::MemCopy(&valueU64, &value, sizeof(value));

			inputs.push_back(Input::GenericInput
				{
					static_cast<u64>(controllerIndex),
					InputDeviceTypes::Controller,
					inputType,
					static_cast<u64>(id),
					valueU64
				});
		}
	}
}
#endif // #if defined(IS_PLATFORM_WINDOWS) 