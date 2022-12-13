#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include "Input/InputDevices/IInputDevice.h"
#include "Input/XInputManager.h"

#include <array>
#include <vector>

namespace Insight
{
	namespace Input
	{
		class InputDevice_KeyboardMouse;
		class InputDevice_Controller;

		class IS_INPUT InputSystem : public Core::Singleton<InputSystem>,  public Core::ISystem
		{
		public:
			InputSystem();
			virtual ~InputSystem() override;

			IS_SYSTEM(InputSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			InputDevice_KeyboardMouse* GetKeyboardMouseDevice() const;
			InputDevice_Controller* GetController(u32 index) const;
			std::vector<IInputDevice*> GetAllInputDevices() const;

			void UpdateInputs(std::vector<GenericInput> inputs);
			void Update(float const deltaTime);
			void ClearFrame();

		private:
			void AddInputDevice(InputDeviceTypes deviceType, u32 id);
			void RemoveInputDevice(InputDeviceTypes deviceType, u32 id);

		private:
			std::vector<IInputDevice*> m_inputDevices;
#ifdef IS_PLATFORM_WINDOWS
			XInputManager m_xinputManager;
			friend class XInputManager;
#endif
		};
	}
}