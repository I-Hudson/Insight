#include "Input/InputSystem.h"
#include "Core/Memory.h"

#include "Input/InputDevices/InputDevice_KeyboardMouse.h"
#include "Input/InputDevices/InputDeivce_Controller.h"

#include "Input/WindowsGamingManager.h"
#include "Input/XInputManager.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"

namespace Insight
{
	namespace Input
	{
		const char* InputSystemInputManagerTypeToString(InputSystemInputManagerTypes type)
		{
			switch (type)
			{
			case Insight::Input::InputSystemInputManagerTypes::XInput:   return "XInput";
			case Insight::Input::InputSystemInputManagerTypes::CppWinRT: return "CppWinRT";
			case Insight::Input::InputSystemInputManagerTypes::NumInputSystemInputManagers:
				break;
			default:
				break;
			}
			FAIL_ASSERT();
			return "";
		}

		InputSystem::InputSystem()
		{
		}

		InputSystem::~InputSystem()
		{
		}

		void InputSystem::Initialise()
		{
			IS_PROFILE_FUNCTION();
#ifdef IS_PLATFORM_WINDOWS
			m_xinputManager = ::New<XInputManager>();
#ifdef IS_CPP_WINRT
			m_windowsGamingManager = ::New<WindowsGamingManager>();
			SetInputManagerType(InputSystemInputManagerTypes::CppWinRT);
#else
			SetInputManager(InputSystemInputManagers::XInput);
#endif // #ifdef IS_CPP_WINRT
#endif // #ifdef IS_PLATFORM_WINDOWS

#ifdef IS_PLATFORM_DESKTOP
			AddInputDevice(InputDeviceTypes::KeyboardMouse, 0);
#endif // IS_PLATFORM_DESKTOP


			m_state = Core::SystemStates::Initialised;
		}

		void InputSystem::Shutdown()
		{
			IS_PROFILE_FUNCTION();

			for (auto& device : m_inputDevices)
			{
				device->Shutdown();
				Delete(device);
			}
			m_inputDevices.clear();

#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			if (m_inputManagerType == InputSystemInputManagerTypes::CppWinRT)
			{
				m_windowsGamingManager->Shutdown();
			}
			::Delete(m_windowsGamingManager);
#endif // #ifdef IS_CPP_WINRT
			if (m_inputManagerType == InputSystemInputManagerTypes::XInput)
			{
				m_xinputManager->Shutdown();
			}
			::Delete(m_xinputManager);
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
			u32 idx = 0;
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

		void InputSystem::SetInputManagerType(InputSystemInputManagerTypes inputManager)
		{
			if (m_inputManagerType == inputManager)
			{
				return;
			}

			// Shutdown the previous input manager used.
			switch (m_inputManagerType)
			{
#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			case InputSystemInputManagerTypes::CppWinRT:
			{
				m_windowsGamingManager->Shutdown();
				break;
			}
#endif // #ifdef IS_CPP_WINRT
			case InputSystemInputManagerTypes::XInput:
			{
				m_xinputManager->Shutdown();
				break;
			}
#endif // #ifdef IS_PLATFORM_WINDOWS
			default:
				break;
			}

			m_inputManagerType = inputManager;

			switch (m_inputManagerType)
			{
#ifdef IS_PLATFORM_WINDOWS
#ifdef IS_CPP_WINRT
			case InputSystemInputManagerTypes::CppWinRT:
			{
				m_windowsGamingManager->Initialise(this);
				break;
			}
#endif // #ifdef IS_CPP_WINRT
			case InputSystemInputManagerTypes::XInput:
			{
				m_xinputManager->Initialise(this);
				break;
			}
#endif // #ifdef IS_PLATFORM_WINDOWS
			default:
				break;
			}
		}

		std::vector<IInputDevice*> InputSystem::GetAllInputDevices() const
		{
			return m_inputDevices;
		}

		IInputDevice* InputSystem::GetLastUsedInputDevices() const
		{
			return m_lastUsedInputDeivce;
		}

		InputSystemInputManagerTypes InputSystem::GetInputManagerType() const
		{
			return m_inputManagerType;
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
			if (m_inputManagerType == InputSystemInputManagerTypes::CppWinRT)
			{
				m_windowsGamingManager->Update(deltaTime);
			}
#endif // #ifdef IS_CPP_WINRT
			if (m_inputManagerType == InputSystemInputManagerTypes::XInput)
			{
				m_xinputManager->Update(deltaTime);
			}
#endif // #ifdef IS_PLATFORM_WINDOWS

			for (auto& device : m_inputDevices)
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
				newInputDevice = New<InputDevice_KeyboardMouse, Core::MemoryAllocCategory::Input>();
				break;
			}
			case Insight::Input::InputDeviceTypes::Controller:
			{
				newInputDevice = New<InputDevice_Controller, Core::MemoryAllocCategory::Input>();
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