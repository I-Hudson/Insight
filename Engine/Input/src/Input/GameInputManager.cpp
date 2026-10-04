#if defined(IS_PLATFORM_WINDOWS) && defined(IS_GAME_INPUT)
#include "Input/GameInputManager.h"
#include "Input/XInputManager.h"
#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDeivce_Controller.h"

#include "Core/Logger.h"
#include "Algorithm/Vector.h"

#include <unordered_map>

namespace Insight
{
	namespace Input
	{
		const std::unordered_map<GameInputNS::GameInputLabel, ControllerButtons> GAME_INPUT_GAME_CONTROLLER_BUTTON_LABEL_TO_INTERNAL =
		{
			{ GameInputNS::GameInputLabel::GameInputLabelUnknown					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNone						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxGuide					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxBack					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxStart					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxMenu					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxView					, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxA						, ControllerButtons::A },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxB						, ControllerButtons::B },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxX						, ControllerButtons::X },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxY						, ControllerButtons::Y },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxDPadUp					, ControllerButtons::DPad_Up },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxDPadDown				, ControllerButtons::DPad_Down },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxDPadLeft				, ControllerButtons::DPad_Left },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxDPadRight				, ControllerButtons::DPad_Right },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxLeftShoulder			, ControllerButtons::Bummer_Left },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxLeftTrigger			, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxLeftStickButton		, ControllerButtons::Thumbstick_Left },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxRightShoulder			, ControllerButtons::Bummber_Right },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxRightTrigger			, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxRightStickButton		, ControllerButtons::Thumbstick_Right },

			{ GameInputNS::GameInputLabel::GameInputLabelXboxPaddle1				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxPaddle2				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxPaddle3				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelXboxPaddle4				, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelLetterA					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterB					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterC					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterD					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterE					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterF					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterG					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterH					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterI					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterJ					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterK					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterL					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterM					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterN					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterO					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterP					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterQ					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterR					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterS					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterT					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterU					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterV					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterW					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterX					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterY					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLetterZ					, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelNumber0					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber1					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber2					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber3					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber4					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber5					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber6					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber7					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber8					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelNumber9					, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelArrowUp					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowUpRight				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowRight					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowDownRight				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowDown					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowDownLLeft				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowLeft					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowUpLeft				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowUpDown				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowLeftRight				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowUpDownLeftRight		, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowClockwise				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowCounterClockwise		, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelArrowReturn				, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconBranding				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconHome					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconMenu					, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconCross					, ControllerButtons::A },
			{ GameInputNS::GameInputLabel::GameInputLabelIconCircle					, ControllerButtons::B },
			{ GameInputNS::GameInputLabel::GameInputLabelIconSquare					, ControllerButtons::X },
			{ GameInputNS::GameInputLabel::GameInputLabelIconTriangle				, ControllerButtons::Y },
			{ GameInputNS::GameInputLabel::GameInputLabelIconStar					, ControllerButtons::Y },

			{ GameInputNS::GameInputLabel::GameInputLabelIconDPadUp					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconDPadDown				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconDPadLeft				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconDPadRight				, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconDialClockwise			, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconDialCounterClockwise	, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconSliderLeftRight		, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconSliderUpDown			, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconWheelUpDown			, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelIconPlus					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconMinus					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelIconSuspension				, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelHome   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelGuide  					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelMode   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelSelect 					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelMenu   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelView   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelBack   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelStart  					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelOptions					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelShare  					, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelUp     					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelDown   					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLeft 						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelRight						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelLB   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelLT   						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelLSB  						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelL1   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelL2   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelL3   						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelRB   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelRT   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelRSB  						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelR1   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelR2   						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelR3   						, ControllerButtons::Unknown },

			{ GameInputNS::GameInputLabel::GameInputLabelPaddleLeft1				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelPaddleLeft2				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelPaddleRight1				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputLabel::GameInputLabelPaddleRight2				, ControllerButtons::Unknown },
		};

		const std::unordered_map<GameInputNS::GameInputGamepadButtons, ControllerButtons> GAMING_INPUT_GAMEPAD_TO_INTERNAL =
		{
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadNone							, ControllerButtons::Unknown },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadMenu							, ControllerButtons::Start },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadView							, ControllerButtons::Select },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadA								, ControllerButtons::A },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadB								, ControllerButtons::B },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadX								, ControllerButtons::X },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadY								, ControllerButtons::Y },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadDPadUp							, ControllerButtons::DPad_Up },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadDPadDown						, ControllerButtons::DPad_Down },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadDPadLeft						, ControllerButtons::DPad_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadDPadRight						, ControllerButtons::DPad_Right },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftShoulder					, ControllerButtons::Bummer_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightShoulder					, ControllerButtons::Bummber_Right },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftTriggerButton				, ControllerButtons::Unknown },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightTriggerButton				, ControllerButtons::Unknown },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftThumbstick					, ControllerButtons::Thumbstick_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftThumbstickUp				, ControllerButtons::Thumbstick_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftThumbstickDown				, ControllerButtons::Thumbstick_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftThumbstickLeft				, ControllerButtons::Thumbstick_Left },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadLeftThumbstickRight				, ControllerButtons::Thumbstick_Left },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightThumbstick					, ControllerButtons::Thumbstick_Right },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightThumbstickUp				, ControllerButtons::Thumbstick_Right },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightThumbstickDown				, ControllerButtons::Thumbstick_Right },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightThumbstickLeft				, ControllerButtons::Thumbstick_Right },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadRightThumbstickRight			, ControllerButtons::Thumbstick_Right },

			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadPaddleLeft1						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadPaddleLeft2						, ControllerButtons::Unknown },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadPaddleRight1					, ControllerButtons::Unknown },
			{ GameInputNS::GameInputGamepadButtons::GameInputGamepadPaddleRight2					, ControllerButtons::Unknown },
		};

		GameInputManager::GameInputManager()
		{ }

		GameInputManager::~GameInputManager()
		{
			for (size_t i = 0; i < m_connectedDevices.size(); ++i)
			{
				m_connectedDevices[i]->Release();
				m_connectedDevices[i] = nullptr;
			}
			m_connectedDevices.clear();
		}

		bool GameInputManager::Initialise(InputSystem* inputSystem)
		{
			m_inputSystem = inputSystem;

			m_connectedDevices.reserve(4);
			for (size_t i = 0; i < m_connectedDevices.size(); ++i)
			{
				m_connectedDevices[i]->Release();
				m_connectedDevices[i] = nullptr;
			}

			HRESULT hr = GameInputNS::GameInputCreate(&m_gameInput);
			if (FAILED(hr))
			{
				return false;
			}

			const GameInputNS::GameInputKind gameInputKind = GameInputNS::GameInputKind::GameInputKindControllerAxis 
				| GameInputNS::GameInputKind::GameInputKindControllerButton 
				| GameInputNS::GameInputKind::GameInputKindControllerSwitch
				| GameInputNS::GameInputKind::GameInputKindGamepad 
				| GameInputNS::GameInputKind::GameInputKindKeyboard 
				| GameInputNS::GameInputKind::GameInputKindMouse 
				| GameInputNS::GameInputKind::GameInputKindSensors
				| GameInputNS::GameInputKind::GameInputKindArcadeStick 
				| GameInputNS::GameInputKind::GameInputKindFlightStick 
				| GameInputNS::GameInputKind::GameInputKindGamepad 
				| GameInputNS::GameInputKind::GameInputKindRacingWheel;

			m_gameInput->RegisterDeviceCallback(nullptr, gameInputKind,
				GameInputNS::GameInputDeviceStatus::GameInputDeviceAnyStatus, GameInputNS::GameInputEnumerationKind::GameInputAsyncEnumeration,
				this, OnDeviceCallback, &m_onDeviceCallbackToken);

			return true;
		}

		void GameInputManager::Shutdown()
		{
			if (m_onDeviceCallbackToken != 0)
			{
				m_gameInput->UnregisterCallback(m_onDeviceCallbackToken);
				m_onDeviceCallbackToken = 0;
			}

			for (size_t i = 0; i < m_connectedDevices.size(); ++i)
			{
				GameInputNS::IGameInputDevice* device = m_connectedDevices[i];
				if (device)
				{
					m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, reinterpret_cast<u64>(device));
					device->Release();
				}
			}

			m_connectedDevices.clear();
		}

		void GameInputManager::Update(float const deltaTime)
		{
			IS_UNUSED(deltaTime);

			for (u64 i = 0; i < m_connectedDevices.size(); ++i)
			{
				auto controller = m_connectedDevices[i];
				if (controller == nullptr)
				{
					continue;
				}
				ProcessInput(static_cast<u32>(i));
				ProcessVibration(static_cast<u32>(i));
			}
		}

		void GameInputManager::OnDeviceCallback(GameInputNS::GameInputCallbackToken callbackToken, void* context, GameInputNS::IGameInputDevice* device, u64 timestamp,
			GameInputNS::GameInputDeviceStatus currentStatus, GameInputNS::GameInputDeviceStatus previousStatus)
		{
			const GameInputNS::GameInputDeviceInfo* deviceInfo;
			HRESULT hr = device->GetDeviceInfo(&deviceInfo);	
			ASSERT(SUCCEEDED(hr) && deviceInfo != nullptr);

			GameInputManager* gameInputManager = static_cast<GameInputManager*>(context);
			ASSERT(gameInputManager);

			if (deviceInfo->supportedInput & GameInputNS::GameInputKindGamepad)
			{
				if (currentStatus == GameInputNS::GameInputDeviceStatus::GameInputDeviceConnected
					&& previousStatus == GameInputNS::GameInputDeviceStatus::GameInputDeviceNoStatus)
				{
					ASSERT(!Algorithm::VectorContains(gameInputManager->m_connectedDevices, device));
					gameInputManager->m_connectedDevices.push_back(device);
					gameInputManager->m_inputSystem->AddInputDevice(InputDeviceTypes::Controller, reinterpret_cast<u64>(device));
					gameInputManager->ExtractDeviceInfo(gameInputManager->m_connectedDevices.size() - 1);
					device->AddRef();
				}
				else if (currentStatus == GameInputNS::GameInputDeviceStatus::GameInputDeviceNoStatus
					&& previousStatus == GameInputNS::GameInputDeviceStatus::GameInputDeviceConnected)
				{
					ASSERT(Algorithm::VectorContains(gameInputManager->m_connectedDevices, device));
					Algorithm::VectorRemove(gameInputManager->m_connectedDevices, device);
					gameInputManager->m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, reinterpret_cast<u64>(device));

				}
			}
		}

		void GameInputManager::ExtractDeviceInfo(u32 const controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}

			GameInputNS::IGameInputDevice* gameInputDevice = m_connectedDevices[controllerIndex];

			const GameInputNS::GameInputDeviceInfo* deviceInfo;
			HRESULT hr = gameInputDevice->GetDeviceInfo(&deviceInfo);
			ASSERT(SUCCEEDED(hr) && deviceInfo != nullptr);

			device->m_deviceInfo.VendorId = static_cast<u32>(deviceInfo->vendorId);
			device->m_deviceInfo.ProductId = static_cast<u32>(deviceInfo->productId);
			device->m_deviceInfo.RevisionId = static_cast<u32>(deviceInfo->revisionNumber);

			device->m_vendor = VendorIdToControllerVendor(device->m_deviceInfo.VendorId);
			device->m_subType = ProductIdToControllerSubType(device->m_deviceInfo.ProductId);
		}

		void GameInputManager::ProcessInput(u32 const controllerIndex)
		{
			std::vector<Input::GenericInput> inputs;
			GameInputNS::IGameInputDevice* gameInputDevice = m_connectedDevices[controllerIndex];

			const GameInputNS::GameInputDeviceInfo* deviceInfo;
			HRESULT hr = gameInputDevice->GetDeviceInfo(&deviceInfo);
			ASSERT(SUCCEEDED(hr) && deviceInfo != nullptr);

			if (deviceInfo->supportedInput & GameInputNS::GameInputKind::GameInputKindGamepad)
			{
				GameInputNS::IGameInputReading* reading;
				HRESULT hr = m_gameInput->GetCurrentReading(GameInputNS::GameInputKind::GameInputKindGamepad, gameInputDevice, &reading);
				auto inputKind = reading->GetInputKind();

				GameInputNS::GameInputGamepadState gamepadState;
				hr = reading->GetGamepadState(&gamepadState);

				reading->Release();

				u32 const gamePadButtons = static_cast<u32>(gamepadState.buttons);

				for (auto const& iter : GAMING_INPUT_GAMEPAD_TO_INTERNAL)
				{
					u32 mask = static_cast<u32>(iter.first);
					ControllerButtons controllerButton = iter.second;
					int buttonState = gamePadButtons & mask;

					inputs.push_back(
						Input::GenericInput
						{
							reinterpret_cast<u64>(gameInputDevice),
							InputDeviceTypes::Controller,
							InputTypes::Button,
							static_cast<u64>(controllerButton),
							static_cast<u64>(buttonState == 0 ? ButtonStates::Released : ButtonStates::Pressed),
							static_cast<u64>(0)
						});
				}

				const float thumbstickDeadzone = 0.1f;
				const float triggerDeadzone = 0.1f;

				XInputManager::ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Left_X), static_cast<u32>(ControllerThumbsticks::Left_Y),
					static_cast<float>(gamepadState.leftThumbstickX), static_cast<float>(gamepadState.leftThumbstickY), thumbstickDeadzone, 1.0f);

				XInputManager::ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Right_X), static_cast<u32>(ControllerThumbsticks::Right_Y),
					static_cast<float>(gamepadState.rightThumbstickX), static_cast<float>(gamepadState.rightThumbstickY), thumbstickDeadzone, 1.0f);

				XInputManager::TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Left), static_cast<float>(gamepadState.leftTrigger),
					triggerDeadzone, 1.0f);
				XInputManager::TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Right), static_cast<float>(gamepadState.rightTrigger),
					triggerDeadzone, 1.0f);
			}

			m_inputSystem->UpdateInputs(inputs);
		}
		

		void GameInputManager::ProcessVibration(u32 const controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[GameInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}

			GameInputNS::IGameInputDevice* gameInputDevice = m_connectedDevices[controllerIndex];

			const GameInputNS::GameInputDeviceInfo* deviceInfo;
			HRESULT hr = gameInputDevice->GetDeviceInfo(&deviceInfo);
			ASSERT(SUCCEEDED(hr) && deviceInfo != nullptr);

			if (deviceInfo->supportedInput == GameInputNS::GameInputKind::GameInputKindGamepad)
			{
				GameInputNS::GameInputRumbleParams state = {};
				state.lowFrequency = device->GetRumbleValue(ControllerRumbles::Left);
				state.highFrequency = device->GetRumbleValue(ControllerRumbles::Right);
				state.leftTrigger = device->GetRumbleValue(ControllerRumbles::LeftTrigger);
				state.rightTrigger = device->GetRumbleValue(ControllerRumbles::RightTrigger);
				gameInputDevice->SetRumbleState(&state);
			}
		}
	}
}

#endif // #if defined(IS_PLATFORM_WINDOWS) && defined(IS_GAME_INPUT)