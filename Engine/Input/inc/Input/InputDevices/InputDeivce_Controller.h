#pragma once

#include "Input/InputDevices/IInputDevice.h"
#include "Input/InputStates/InputButtonState.h"
#include "Input/InputStates/InputThumbstickState.h"
#include "Input/InputStates/InputTriggerState.h"
#include "Input/InputStates/InputRumbleState.h"
#include "Input/ControllerTypes.h"

namespace Insight
{
	namespace Input
	{
		class XInputManager;
		class WindowsGamingManager;

		enum class ControllerButtons : u16
		{
			Unknown,
			A,
			B, 
			X, 
			Y,
			Thumbstick_Left,
			Thumbstick_Right,
			DPad_Up,
			DPad_Right,
			DPad_Down,
			DPad_Left,
			Bummer_Left,
			Bummber_Right,
			Start,
			Select,
			Share,

			NumButtons
		};
		IS_INPUT const char* ControllerButtonsToString(ControllerButtons button);

		enum class ControllerThumbsticks : u16
		{
			Left_X,
			Left_Y,
			Right_X,
			Right_Y,

			NumThumbsticks
		};
		IS_INPUT const char* ControllerThumbstickToString(ControllerThumbsticks thumbstick);

		enum class ControllerTriggers : u16
		{
			Left,
			Right,

			NumTriggers
		};
		IS_INPUT const char* ControllerTriggerToString(ControllerTriggers trigger);

		enum class ControllerRumbles : u16
		{
			Left,
			Right,
			LeftTrigger,
			RightTrigger,

			NumRumbles
		};
		IS_INPUT const char* ControllerRumbleToString(ControllerRumbles rumble);

		/// <summary>
		/// Controller based on Xbox one controller.
		/// </summary>
		class IS_INPUT InputDevice_Controller : public IInputDevice
		{
		public:
			InputDevice_Controller();
			virtual ~InputDevice_Controller() override = default;

			virtual void Initialise(u32 id) override;
			virtual void Shutdown() override;

			virtual InputDeviceTypes GetDeviceType() const override { return InputDeviceTypes::Controller; }

			virtual void ProcessInput(GenericInput const& input) override;
			virtual bool HasInput() const override;
			virtual void Update(float const deltaTime) override;
			virtual void ClearFrame() override;

			ControllerVendors GetVendor() const;
			ControllerSubTypes GetSubType() const;

			bool WasPressed(ControllerButtons buttonIdx) const;
			bool WasReleased(ControllerButtons buttonIdx) const;
			bool WasHeld(ControllerButtons buttonIdx) const;

			float GetThumbstickValue(ControllerThumbsticks tumbstick) const;
			float GetTriggerValue(ControllerTriggers trigger) const;

			void SetRumbleValue(ControllerRumbles rumble, float value);
			float GetRumbleValue(ControllerRumbles rumble) const;

		private:
			InputButtonState<static_cast<u64>(ControllerButtons::NumButtons)> m_buttons;
			InputThumbstickState m_leftTumbstick;
			InputThumbstickState m_rightTumbstick;
			InputTriggerState m_leftTrigger;
			InputTriggerState m_rightTrigger;
			std::array<InputRumbleState, static_cast<u64>(ControllerRumbles::NumRumbles)> m_rumbles;

			ControllerVendors m_vendor = ControllerVendors::Unknown;
			ControllerSubTypes m_subType = ControllerSubTypes::Unknown;

#ifdef IS_PLATFORM_WINDOWS
			friend class WindowsGamingManager;
			friend class XInputManager;
#endif // #ifdef IS_PLATFORM_WINDOWS
		};
	}
}