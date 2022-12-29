#include "Input/InputSystem.h"
#include "Core/Memory.h"

#include "Input/InputDevices/InputDevice_KeyboardMouse.h"
#include "Input/InputDevices/InputDeivce_Controller.h"

#include "Core/Logger.h"

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
#ifdef IS_PLATFORM_DESKTOP
			AddInputDevice(InputDeviceTypes::KeyboardMouse, 0);
#endif // IS_PLATFORM_DESKTOP

#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			m_windowsGamingManager.Initialise(this);
#else
			m_xinputManager.Initialise(this);
#endif // #ifdef IS_CPP_WINRT
#endif // #ifdef IS_PLATFORM_WINDOWS

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
#ifdef IS_CPP_WINRT
			m_windowsGamingManager.Shutdown();
#else
			m_xinputManager.Shutdown();
#endif // #ifdef IS_CPP_WINRT
#endif // IS_PLATFORM_WINDOWS

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

		InputDevice_Controller* InputSystem::GetController(u32 index) const
		{
			int idx = 0;
			for (auto& device : m_inputDevices)
			{
				if (device->GetDeviceType() == InputDeviceTypes::Controller)
				{
					if (idx == index)
					{
						return static_cast<InputDevice_Controller*>(device);
					}
					++idx;
				}
			}
			return nullptr;
		}

		std::vector<IInputDevice*> InputSystem::GetAllInputDevices() const
		{
			return m_inputDevices;
		}

		IInputDevice* InputSystem::GetLastUsedInputDevices() const
		{
			return m_lastUsedInputDeivce;
		}

		void InputSystem::UpdateInputs(std::vector<GenericInput> inputs)
		{
			for (auto& device : m_inputDevices)
			{
				for (const auto& input : inputs)
				{
					if (device->GetDeviceType() == input.InputDevice
						&& device->GetId() == input.DevieId)
					{
						device->ProcessInput(input);
					}
				}
			}
		}

		void InputSystem::Update(float const deltaTime)
		{
#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			m_windowsGamingManager.Update(deltaTime);
#else
			m_xinputManager.Update(deltaTime);
#endif // #ifdef IS_CPP_WINRT
#endif // #ifdef IS_PLATFORM_WINDOWS

			for (auto& device: m_inputDevices)
			{
				device->Update(deltaTime);
				if (device->HasInput())
				{
					m_lastUsedInputDeivce = device;
				}
			}
		}

		void InputSystem::ClearFrame()
		{
			for (auto& device : m_inputDevices)
			{
				device->ClearFrame();
			}
		}


		IInputDevice* InputSystem::AddInputDevice(InputDeviceTypes deviceType, u32 id)
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
				newInputDevice = New<InputDevice_Controller>();
				break;
			}
			}
			ASSERT(newInputDevice);
			newInputDevice->Initialise(id);
			m_inputDevices.push_back(newInputDevice);
			return newInputDevice;
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

					if (device == m_lastUsedInputDeivce)
					{
						m_lastUsedInputDeivce = nullptr;
					}

					Delete(device);
					m_inputDevices.erase(m_inputDevices.begin() + idx);
					break;
				}
				++idx;
			}
		}
	}
}