#if defined(IS_PLATFORM_WINDOWS) && defined(IS_CPP_WINRT)
#include "Input/WindowsGamingManager.h"
#include "Input/XInputManager.h"
#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDeivce_Controller.h"

#include "Core/Logger.h"

#include <unordered_map>

namespace Insight
{
	namespace Input
	{
		const std::unordered_map<winrt::Windows::Gaming::Input::GameControllerButtonLabel, ControllerButtons> WINDOWS_GAMING_INPUT_GAME_CONTROLLER_BUTTON_LABEL_TO_INTERNAL =
		{
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::None					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxBack				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxStart				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxMenu				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxView				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxUp					, ControllerButtons::DPad_Up },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxDown				, ControllerButtons::DPad_Down },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxLeft				, ControllerButtons::DPad_Left },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxRight				, ControllerButtons::DPad_Right },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxA					, ControllerButtons::A },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxB					, ControllerButtons::B },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxX					, ControllerButtons::X },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxY					, ControllerButtons::Y },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxLeftBumper			, ControllerButtons::Bummer_Left },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxLeftTrigger			, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxLeftStickButton		, ControllerButtons::Thumbstick_Left },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxRightBumper			, ControllerButtons::Bummber_Right },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxRightTrigger		, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxRightStickButton	, ControllerButtons::Thumbstick_Right },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxPaddle1				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxPaddle2				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxPaddle3				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::XboxPaddle4				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Mode					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Select					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Menu					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::View					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Back					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Start					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Options					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Share					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Up						, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Down					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Left					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Right					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterA					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterB					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterC					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterL					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterR					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterX					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterY					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterZ					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LetterR					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Cross					, ControllerButtons::A },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Circle					, ControllerButtons::B },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Square					, ControllerButtons::X },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Triangle				, ControllerButtons::Y },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LeftBumper				, ControllerButtons::Bummer_Left },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LeftTrigger				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::LeftStickButton			, ControllerButtons::Thumbstick_Left },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Left1					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Left2					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Left3					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::RightBumper				, ControllerButtons::Bummber_Right },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::RightTrigger			, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::RightStickButton		, ControllerButtons::Thumbstick_Right },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Right1					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Right2					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Right3					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Paddle1					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Paddle2					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Paddle3					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Paddle4					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Plus					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Minus					, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::DownLeftArrow			, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::DialLeft				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::DialRight				, ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GameControllerButtonLabel::Suspension				, ControllerButtons::Unknown },
		};

		const std::unordered_map<winrt::Windows::Gaming::Input::GamepadButtons, ControllerButtons> WINDOWS_GAMING_INPUT_GAMEPAD_TO_INTERNAL =
		{
			{ winrt::Windows::Gaming::Input::GamepadButtons::None           , ControllerButtons::Unknown },
			{ winrt::Windows::Gaming::Input::GamepadButtons::Menu           , ControllerButtons::Start },
			{ winrt::Windows::Gaming::Input::GamepadButtons::View           , ControllerButtons::Select },
			{ winrt::Windows::Gaming::Input::GamepadButtons::A              , ControllerButtons::A },
			{ winrt::Windows::Gaming::Input::GamepadButtons::B              , ControllerButtons::B },
			{ winrt::Windows::Gaming::Input::GamepadButtons::X              , ControllerButtons::X },
			{ winrt::Windows::Gaming::Input::GamepadButtons::Y              , ControllerButtons::Y },
			{ winrt::Windows::Gaming::Input::GamepadButtons::DPadUp         , ControllerButtons::DPad_Up },
			{ winrt::Windows::Gaming::Input::GamepadButtons::DPadDown       , ControllerButtons::DPad_Down },
			{ winrt::Windows::Gaming::Input::GamepadButtons::DPadLeft       , ControllerButtons::DPad_Left },
			{ winrt::Windows::Gaming::Input::GamepadButtons::DPadRight      , ControllerButtons::DPad_Right },
			{ winrt::Windows::Gaming::Input::GamepadButtons::LeftShoulder   , ControllerButtons::Bummer_Left },
			{ winrt::Windows::Gaming::Input::GamepadButtons::RightShoulder  , ControllerButtons::Bummber_Right },
			{ winrt::Windows::Gaming::Input::GamepadButtons::LeftThumbstick , ControllerButtons::Thumbstick_Left },
			{ winrt::Windows::Gaming::Input::GamepadButtons::RightThumbstick, ControllerButtons::Thumbstick_Right },
		};

		WindowsGamingManager::WindowsGamingManager()
		{ }

		WindowsGamingManager::~WindowsGamingManager()
		{
			for (size_t i = 0; i < m_connectControllers.size(); ++i)
			{
				m_connectControllers[i] = nullptr;
			}
		}

		void WindowsGamingManager::Initialise(InputSystem* inputSystem)
		{
			m_inputSystem = inputSystem;

			for (size_t i = 0; i < m_connectControllers.size(); ++i)
			{
				m_connectControllers[i] = nullptr;
			}

			m_onControllerAddedEvent = winrt::Windows::Gaming::Input::RawGameController::RawGameControllerAdded(
				[this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller)
				{
					OnRawControllerAdded(sender, controller);
				});
			m_onControllerRemovedEvent = winrt::Windows::Gaming::Input::RawGameController::RawGameControllerRemoved(
				[this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller)
				{
					OnRawControllerRemoved(sender, controller);
				});

			auto controllers = winrt::Windows::Gaming::Input::RawGameController::RawGameControllers();
			for (u32 i = 0; i < controllers.Size(); ++i)
			{
				OnRawControllerAdded({ }, controllers.GetAt(i));
			}
		}

		void WindowsGamingManager::Shutdown()
		{
			if (m_onControllerAddedEvent)
			{
				winrt::Windows::Gaming::Input::RawGameController::RawGameControllerAdded(m_onControllerAddedEvent);
				m_onControllerAddedEvent = { 0 };
			}

			if (m_onControllerRemovedEvent)
			{
				winrt::Windows::Gaming::Input::RawGameController::RawGameControllerRemoved(m_onControllerRemovedEvent);
				m_onControllerRemovedEvent = { 0 };
			}

			for (size_t i = 0; i < m_connectControllers.size(); ++i)
			{
				if (m_connectControllers[i])
				{
					m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, static_cast<u32>(i));
					m_connectControllers[i] = nullptr;
				}
			}
		}

		void WindowsGamingManager::Update(float const deltaTime)
		{
			for (u64 i = 0; i < m_connectControllers.size(); ++i)
			{
				auto controller = m_connectControllers[i];
				if (controller == nullptr)
				{
					continue;
				}
				ProcessInput(static_cast<u32>(i));
				ProcessVibration(static_cast<u32>(i));
			}
		}

		void WindowsGamingManager::OnRawControllerAdded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller)
		{
			for (u32 i = 0; i < m_connectControllers.size(); ++i)
			{
				if (m_connectControllers[static_cast<u64>(i)] == controller)
				{
					// Controller already tracked.
					return;
				}

				if (m_connectControllers[static_cast<u64>(i)] == nullptr)
				{
					m_connectControllers[i] = controller;
					m_inputSystem->AddInputDevice(InputDeviceTypes::Controller, i);
					ExtractDeviceInfo(i);
					break;
				}
			}
		}

		void WindowsGamingManager::OnRawControllerRemoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller)
		{
			for (u32 i = 0; i < m_connectControllers.size(); ++i)
			{
				if (m_connectControllers[static_cast<u64>(i)] == controller)
				{
					m_connectControllers[static_cast<u64>(i)] = nullptr;
					m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, i);
					break;
				}
			}
		}

		void WindowsGamingManager::ExtractDeviceInfo(u32 const controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}

			auto& rawController = m_connectControllers[controllerIndex];
			auto gamepad = winrt::Windows::Gaming::Input::Gamepad::FromGameController(rawController);

			device->m_deviceInfo.VendorId = static_cast<u32>(rawController.HardwareVendorId());
			device->m_deviceInfo.ProductId = static_cast<u32>(rawController.HardwareProductId());
			device->m_deviceInfo.RevisionId = static_cast<u32>(0ul);

			device->m_vendor = VendorIdToControllerVendor(device->m_deviceInfo.VendorId);
			device->m_subType = ProductIdToControllerSubType(device->m_deviceInfo.ProductId);
		}

		void WindowsGamingManager::ProcessInput(u32 const controllerIndex)
		{
			std::vector<Input::GenericInput> inputs;
			winrt::Windows::Gaming::Input::Gamepad gamepad = winrt::Windows::Gaming::Input::Gamepad::FromGameController(m_connectControllers[controllerIndex]);
			if (gamepad != nullptr)
			{
				winrt::Windows::Gaming::Input::GamepadReading gamepadReading = gamepad.GetCurrentReading();
				u32 const gamePadButtons = static_cast<u32>(gamepadReading.Buttons);

				for (auto const& iter : WINDOWS_GAMING_INPUT_GAMEPAD_TO_INTERNAL)
				{
					u32 mask = static_cast<u32>(iter.first);
					ControllerButtons controllerButton = iter.second;
					int buttonState = gamePadButtons & mask;

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

				const float thumbstickDeadzone = 0.1f;
				const float triggerDeadzone = 0.1f;

				XInputManager::ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Left_X), static_cast<u32>(ControllerThumbsticks::Left_Y),
					static_cast<float>(gamepadReading.LeftThumbstickX), static_cast<float>(gamepadReading.LeftThumbstickY), thumbstickDeadzone, 1.0f);
				XInputManager::ThumbstickInput(inputs, controllerIndex, InputTypes::Thumbstick, static_cast<u32>(ControllerThumbsticks::Right_X), static_cast<u32>(ControllerThumbsticks::Right_Y),
					static_cast<float>(gamepadReading.RightThumbstickX), static_cast<float>(gamepadReading.RightThumbstickY), thumbstickDeadzone, 1.0f);

				XInputManager::TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Left), static_cast<float>(gamepadReading.LeftTrigger),
					triggerDeadzone, 1.0f);
				XInputManager::TriggerInput(inputs, controllerIndex, InputTypes::Trigger, static_cast<u32>(ControllerTriggers::Right), static_cast<float>(gamepadReading.RightTrigger),
					triggerDeadzone, 1.0f);
			}
			else
			{
				IS_LOG_CORE_WARN("[WindowsGamingManager::ProcessInput] Controller at index '{}' was unable to create winrt::Windows::Gaming::Input::Gamepad. Falling back to RawGameController.", controllerIndex);
				const winrt::Windows::Gaming::Input::RawGameController& rawGameController = m_connectControllers[controllerIndex];

				std::unique_ptr<bool[]> buttonReading = std::make_unique<bool[]>(rawGameController.ButtonCount());

				std::vector<winrt::Windows::Gaming::Input::GameControllerSwitchPosition> switchPositionReading;
				switchPositionReading.resize(rawGameController.SwitchCount());

				std::vector<double> axisReading;
				axisReading.resize(rawGameController.AxisCount());

				const u64 getCurrentReadingRCode = rawGameController.GetCurrentReading(winrt::array_view<bool>(buttonReading.get(), buttonReading.get() + rawGameController.ButtonCount()), switchPositionReading, axisReading);
				for (size_t buttonIdx = 0; buttonIdx < rawGameController.ButtonCount(); ++buttonIdx)
				{
					const winrt::Windows::Gaming::Input::GameControllerButtonLabel buttonLabel = rawGameController.GetButtonLabel(buttonIdx);
					const auto gamebuttonLabelToInteral = WINDOWS_GAMING_INPUT_GAME_CONTROLLER_BUTTON_LABEL_TO_INTERNAL.find(buttonLabel);
					const ControllerButtons controllerButton = 
						gamebuttonLabelToInteral != WINDOWS_GAMING_INPUT_GAME_CONTROLLER_BUTTON_LABEL_TO_INTERNAL.end() ? gamebuttonLabelToInteral->second : ControllerButtons::Unknown;
					const bool buttonState = buttonReading.get()[buttonIdx];

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

				//for (size_t switchIdx = 0; switchIdx < switchPositionReading.size(); ++switchIdx)
				//{
				//	const ControllerButtons controllerButton = ControllerButtons::Unknown;
				//	const winrt::Windows::Gaming::Input::GameControllerSwitchPosition switchState = switchPositionReading[switchIdx];

				//	inputs.push_back(
				//		Input::GenericInput
				//		{
				//			static_cast<u64>(controllerIndex),
				//			InputDeviceTypes::Controller,
				//			InputTypes::Button,
				//			static_cast<u64>(controllerButton),
				//			static_cast<u64>(buttonState == 0 ? ButtonStates::Released : ButtonStates::Pressed),
				//			static_cast<u64>(0)
				//		});
				//}


			}

			m_inputSystem->UpdateInputs(inputs);
		}
		

		void WindowsGamingManager::ProcessVibration(u32 const controllerIndex)
		{
			InputDevice_Controller* device = m_inputSystem->GetController(controllerIndex);
			if (!device)
			{
				IS_LOG_CORE_ERROR("[XInputManager::ProcessVibration] Trying to process controller index '{}', controller at index is not valid.", controllerIndex);
				return;
			}

			winrt::Windows::Gaming::Input::Gamepad gamepad = winrt::Windows::Gaming::Input::Gamepad::FromGameController(m_connectControllers[controllerIndex]);

			if (gamepad)
			{
				winrt::Windows::Gaming::Input::GamepadVibration state = {};
				state.LeftMotor = device->GetRumbleValue(ControllerRumbles::Left);
				state.RightMotor = device->GetRumbleValue(ControllerRumbles::Right);
				state.LeftTrigger = device->GetRumbleValue(ControllerRumbles::LeftTrigger);
				state.RightTrigger = device->GetRumbleValue(ControllerRumbles::RightTrigger);
				gamepad.Vibration(state);
			}
		}
	}
}

#endif // #if defined(IS_PLATFORM_WINDOWS) && defined(IS_CPP_WINRT)