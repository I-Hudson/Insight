#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include "Input/InputDevices/IInputDevice.h"

#include <vector>

namespace Insight
{
	namespace Input
	{
		class InputDevice_KeyboardMouse;
		class InputDevice_Controller;
		class WindowsGamingManager;
		class XInputManager;

		enum class InputSystemInputManagerTypes
		{
			XInput,
			CppWinRT,

			NumInputSystemInputManagers
		};
		IS_INPUT const char* InputSystemInputManagerTypeToString(InputSystemInputManagerTypes type);

		class IS_INPUT InputSystem : public Core::Singleton<InputSystem>,  public Core::ISystem
		{
		public:
			InputSystem();
			virtual ~InputSystem() override;

			IS_SYSTEM(InputSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			void SetInputManagerType(InputSystemInputManagerTypes inputManager);
			InputSystemInputManagerTypes GetInputManagerType() const;

			InputDevice_KeyboardMouse* GetKeyboardMouseDevice() const;
			InputDevice_Controller* GetController(u32 index) const;
			IInputDevice* GetLastUsedInputDevices() const;
			std::vector<IInputDevice*> GetAllInputDevices() const;

			void UpdateInputs(std::vector<GenericInput> inputs);
			void Update(float const deltaTime);
			void ClearFrame();

		private:
			IInputDevice* AddInputDevice(InputDeviceTypes deviceType, u32 id);
			void RemoveInputDevice(InputDeviceTypes deviceType, u32 id);

		private:
			std::vector<IInputDevice*> m_inputDevices;
			IInputDevice* m_lastUsedInputDeivce = nullptr;
			InputSystemInputManagerTypes m_inputManagerType;
#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			WindowsGamingManager* m_windowsGamingManager;
			friend class WindowsGamingManager;
#endif // #ifdef IS_CPP_WINRT
			XInputManager* m_xinputManager;
			friend class XInputManager;
#endif // #ifdef IS_PLATFORM_WINDOWS
		};
	}
}