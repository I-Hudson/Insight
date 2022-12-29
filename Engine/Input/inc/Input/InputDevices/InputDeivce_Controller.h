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
		class XInputManager;
		class WindowsGamingManager;

		// https://devicehunt.com/view/type/usb/vendor/045E#search-results-table
		constexpr u64 MICROSOFT_VENDOR_ID                  = 0x045E;
		constexpr u64 PLAYSTATION_VENDOR_ID                = 0x054C;


		// Xbox controllers
		constexpr u64 XBOX_CONTROLLER_ONE_S                = 0x02EA;
		constexpr u64 XBOX_CONTROLLER_ONE_S_BLUETOOTH      = 0x02FD;
		constexpr u64 XBOX_CONTROLLER_ONE_ELITE            = 0x02E3;
		constexpr u64 XBOX_CONTROLLER_ONE_BLUETOOTH		   = 0xB12;
		constexpr u64 XBOX_CONTROLLER_ONE                  = 0x02D1;
		constexpr u64 XBOX_CONTROLLER_ONE_2015             = 0x02DD;
		constexpr u64 XBOX_CONTROLLER_360                  = 0x045E;

		// PlayStation controllers
		constexpr u64 PLAYSTATION_CONTROLLER_DUALSENSE_PS5 = 0x0CE6;
		constexpr u64 PLAYSTATION_CONTROLLER_PS3           = 0x0268;
		constexpr u64 PLAYSTATION_CONTROLLER_CLASSIC       = 0x0CDA;


		enum class ControllerVendors
		{
			Unknown,
			Microsoft,
			Sony,
			NumControllerVendors
		};
		IS_INPUT const char* ControllerVendorToString(ControllerVendors vendor);

		IS_INPUT ControllerVendors VendorIdToControllerVendor(u32 vendorId);


		/// <summary>
		/// Define the which controller "InputDevice_Controller" is.
		/// Note: This defines the internal representation of the controller, this might not be the 
		/// actual controller type. 
		/// (Example if XInput is used then all controllers are defined as Xbox360 even if they are not Xbox360 controllers).
		/// </summary>
		enum class ControllerSubTypes : u16
		{
			Unknown,
			Xbox360, 
			XboxOne,
			XboxOneS,
			XboxOneElite,

			PlayStationDualSensePS5,
			PlayStationPS3,
			PlayStationClassic,

			NumControllerSubTypes
		};
		IS_INPUT const char* ControllerSubTypeToString(ControllerSubTypes subType);

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

		IS_INPUT ControllerSubTypes DeviceIdToControllerSubType(u32 deviceId);

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