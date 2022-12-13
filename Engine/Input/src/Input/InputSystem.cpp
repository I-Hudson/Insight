#include "Input/InputSystem.h"
#include "Core/Memory.h"

#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

namespace Insight
{
	namespace Input
	{
		InputSystem::InputSystem()
		{
		}

		InputSystem::~InputSystem()
		{
		}

		void InputSystem::Initialise()
		{
			InputDevice_KeyboardMouse* keyboardDevice = New<InputDevice_KeyboardMouse>();
			keyboardDevice->Initialise();
			m_inputDevices.push_back(keyboardDevice);

#ifdef IS_PLATFORM_WINDOWS
			m_xinputManager.Initialise(this);
#endif

			m_state = Core::SystemStates::Initialised;
		}

		void InputSystem::Shutdown()
		{
			for (auto& device : m_inputDevices)
			{
				device->Shutdown();
				Delete(device);
			}
			m_inputDevices.clear();

#ifdef IS_PLATFORM_WINDOWS
			m_xinputManager.Shutdown();
#endif

			m_state = Core::SystemStates::Not_Initialised;
		}

		InputDevice_KeyboardMouse* InputSystem::GetKeyboardMouseDevice() const
		{
			for (auto& device : m_inputDevices)
			{
				if (device->GetDeviceType() == InputDeviceTypes::KeyboardMouse)
				{
					return static_cast<InputDevice_KeyboardMouse*>(device);
				}
			}
			return nullptr;
		}

		void InputSystem::UpdateInputs(std::vector<GenericInput> inputs)
		{
			for (auto& device : m_inputDevices)
			{
				for (const auto& input : inputs)
				{
					if (device->GetDeviceType() == input.InputDevice)
					{
						device->ProcessInput(input);
					}
				}
			}
		}

		void InputSystem::Update(float const deltaTime)
		{
#ifdef IS_PLATFORM_WINDOWS
			m_xinputManager.Update();
#endif
			for (auto& device: m_inputDevices)
			{
				device->Update(deltaTime);
			}
		}
	}
}