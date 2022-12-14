#pragma once

#include "Input/InputDevices/IInputDevice.h"
#include "Input/InputStates/InputButtonState.h"
#include "Input/InputStates/InputThumbstickState.h"
#include "Input/InputStates/InputTriggerState.h"
#include "Input/InputStates/InputRumbleState.h"

namespace Insight
{
	namespace Input
	{
		enum class ControllerButtons : u16
		{
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
		IS_INPUT CONSTEXPR const char* ControllerButtonsToString(ControllerButtons button);

		enum class ControllerThumbsticks : u16
		{
			Left_X,
			Left_Y,
			Right_X,
			Right_Y,

			NumThumbsticks
		};
		IS_INPUT CONSTEXPR const char* ControllerThumbstickToString(ControllerThumbsticks thumbstick);

		enum class ControllerTriggers : u16
		{
			Left,
			Right,

			NumTriggers
		};
		IS_INPUT CONSTEXPR const char* ControllerTriggerToString(ControllerTriggers trigger);

		enum class ControllerRumbles : u16
		{
			Left,
			Right,

			NumRumbles
		};
		IS_INPUT CONSTEXPR const char* ControllerRumbleToString(ControllerRumbles rumble);


		/// <summary>
		/// Controller based on Xbox one controller.
		/// </summary>
		class IS_INPUT InputDevice_Controller : public IInputDevice
		{
		public:
			InputDevice_Controller() = default;
			virtual ~InputDevice_Controller() override = default;

			virtual void Initialise(u32 id) override;
			virtual void Shutdown() override;

			virtual InputDeviceTypes GetDeviceType() const { return InputDeviceTypes::Controller; }

			virtual void ProcessInput(GenericInput const& input) override;
			virtual bool HasInput() const override;
			virtual void Update(float const deltaTime) override;
			virtual void ClearFrame() override;

			bool WasPressed(ControllerButtons buttonIdx) const;
			bool WasReleased(ControllerButtons buttonIdx) const;
			bool WasHeld(ControllerButtons buttonIdx) const;

			float GetThumbstickValue(ControllerThumbsticks tumbstick) const;
			i16 GetThumbstickRawValue(ControllerThumbsticks tumbstick) const;

			float GetTriggerValue(ControllerTriggers trigger) const;
			u8 GetTriggerRawValue(ControllerTriggers trigger) const;

			void SetRumbleValue(ControllerRumbles rumble, float value);
			float GetRumbleValue(ControllerRumbles rumble) const;
			u16 GetRumbleRawValue(ControllerRumbles rumble) const;

		private:
			InputButtonState<static_cast<u64>(ControllerButtons::NumButtons)> m_buttons;
			InputThumbstickState m_leftTumbstick;
			InputThumbstickState m_rightTumbstick;
			InputTriggerState m_leftTrigger;
			InputTriggerState m_rightTrigger;
			std::array<InputRumbleState, static_cast<u64>(ControllerRumbles::NumRumbles)> m_rumbles;
		};
	}
}