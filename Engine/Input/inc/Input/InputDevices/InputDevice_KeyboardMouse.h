#pragma once

#include "Input/InputDevices/IInputDevice.h"
#include "Input/InputButtonState.h"
#include "Input/InputMouseState.h"

namespace Insight
{
	namespace Input
	{
        enum class KeyboardButtons : uint16_t
        {
            Key_A = 0,
            Key_B,
            Key_C,
            Key_D,
            Key_E,
            Key_F,
            Key_G,
            Key_H,
            Key_I,
            Key_J,
            Key_K,
            Key_L,
            Key_M,
            Key_N,
            Key_O,
            Key_P,
            Key_Q,
            Key_R,
            Key_S,
            Key_T,
            Key_U,
            Key_V,
            Key_W,
            Key_X,
            Key_Y,
            Key_Z,
            Key_0,
            Key_1,
            Key_2,
            Key_3,
            Key_4,
            Key_5,
            Key_6,
            Key_7,
            Key_8,
            Key_9,
            Key_Comma,
            Key_Period,
            Key_ForwardSlash,
            Key_SemiColon,
            Key_Quote,
            Key_LBracket,
            Key_RBracket,
            Key_BackSlash,
            Key_Underscore,
            Key_Minus = Key_Underscore,
            Key_Equals,
            Key_Plus = Key_Equals,
            Key_Backspace,
            Key_Tilde,
            Key_Tab,
            Key_CapsLock,
            Key_Enter,
            Key_Escape,
            Key_Space,
            Key_Left,
            Key_Up,
            Key_Right,
            Key_Down,
            Key_NumLock,
            Key_Numpad0,
            Key_Numpad1,
            Key_Numpad2,
            Key_Numpad3,
            Key_Numpad4,
            Key_Numpad5,
            Key_Numpad6,
            Key_Numpad7,
            Key_Numpad8,
            Key_Numpad9,
            Key_NumpadEnter,
            Key_NumpadMultiply,
            Key_NumpadPlus,
            Key_NumpadMinus,
            Key_NumpadPeriod,
            Key_NumpadDivide,
            Key_F1,
            Key_F2,
            Key_F3,
            Key_F4,
            Key_F5,
            Key_F6,
            Key_F7,
            Key_F8,
            Key_F9,
            Key_F10,
            Key_F11,
            Key_F12,
            Key_F13,
            Key_F14,
            Key_F15,
            Key_PrintScreen,
            Key_ScrollLock,
            Key_Pause,
            Key_Insert,
            Key_Delete,
            Key_Home,
            Key_End,
            Key_PageUp,
            Key_PageDown,
            Key_Application,
            Key_LShift,
            Key_RShift,
            Key_LCtrl,
            Key_RCtrl,
            Key_LAlt,
            Key_RAlt,

            NumButtons,
        };

		class IS_INPUT InputDevice_KeyboardMouse : public IInputDevice
		{
		public:
			InputDevice_KeyboardMouse();
			virtual ~InputDevice_KeyboardMouse() override;

			virtual void Initialise(u32 id) override;
			virtual void Shutdown() override;

			virtual InputDeviceTypes GetDeviceType() const { return InputDeviceTypes::KeyboardMouse; }

			virtual void ProcessInput(GenericInput const& input) override;
			virtual void Update(float const deltaTime) override;
            virtual void ClearFrame() override;

			bool WasPressed(KeyboardButtons buttonIdx) const;
			bool WasReleased(KeyboardButtons buttonIdx) const;
			bool WasHeld(KeyboardButtons buttonIdx) const;

            float GetMouseXPosition() const;
            float GetMouseYPosition() const;
            bool WasPressed(MouseButtons buttonIdx) const;
            bool WasReleased(MouseButtons buttonIdx) const;
            bool WasHeld(MouseButtons buttonIdx) const;

		private:
            InputButtonState<static_cast<u64>(KeyboardButtons::NumButtons)> m_buttons;
            InputMouseState m_mouse;
		};
	}
}