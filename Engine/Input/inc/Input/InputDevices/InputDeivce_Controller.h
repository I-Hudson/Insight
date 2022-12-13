#pragma once

#include "Input/InputDevices/IInputDevice.h"
#include "Input/InputButtonState.h"

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
			Joystick_Left,
			Joystick_Right,
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
			virtual void Update(float const deltaTime) override;
			virtual void ClearFrame() override;

			bool WasPressed(ControllerButtons buttonIdx) const;
			bool WasReleased(ControllerButtons buttonIdx) const;
			bool WasHeld(ControllerButtons buttonIdx) const;

		private:
			InputButtonState<static_cast<u64>(ControllerButtons::NumButtons)> m_buttons;
		};
	}
}