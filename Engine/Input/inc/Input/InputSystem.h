#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include "Input/InputDevices/IInputDevice.h"

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

			void UpdateInputs(std::vector<GenericInput> inputs);
			void Update(float const deltaTime);

		private:
			std::vector<IInputDevice*> m_inputDevices;
		};
	}
}