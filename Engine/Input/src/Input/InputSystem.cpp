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
			AddInputDevice(InputDeviceTypes::KeyboardMouse, 0);

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

		void InputSystem::ClearFrame()
		{
			for (auto& device : m_inputDevices)
			{
				device->ClearFrame();
			}
		}


		void InputSystem::AddInputDevice(InputDeviceTypes deviceType, u32 id)
		{
			IInputDevice* newInputDevice = nullptr;
			switch (deviceType)
			{
			case Insight::Input::InputDeviceTypes::KeyboardMouse: 
			{
				newInputDevice = New<InputDevice_KeyboardMouse>();
				break;
			}
			case Insight::Input::InputDeviceTypes::Controller:
			{
				//newInputDevice = New<>();
				break;
			}
			}
			ASSERT(newInputDevice);
			newInputDevice->Initialise(id);
			m_inputDevices.push_back(newInputDevice);
		}

		void InputSystem::RemoveInputDevice(InputDeviceTypes deviceType, u32 id)
		{
			int idx = 0;
			for (auto& device : m_inputDevices)
			{
				if (device->GetDeviceType() == deviceType
					&& device->GetId() == id)
				{
					device->Shutdown();
					Delete(device);
					m_inputDevices.erase(m_inputDevices.begin() + idx);
					break;
				}
				++idx;
			}
		}
	}
}