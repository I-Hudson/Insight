#include "Input/XInputManager.h"

#ifdef IS_PLATFORM_WINDOWS

#include "Input/InputSystem.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

namespace Insight
{
	namespace Input
	{
		void XInputManager::Initialise(InputSystem* inputSystem)
		{
			m_inputSystem = inputSystem;
		}

		void XInputManager::Shutdown()
		{
		}

		void XInputManager::Update()
		{
			DWORD dwResult;
			for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				XINPUT_STATE state;
				Platform::MemSet(&state, 0, sizeof(XINPUT_STATE));

				// Simply get the state of the controller from XInput.
				dwResult = XInputGetState(i, &state);

				if (dwResult == ERROR_SUCCESS)
				{
					// Controller is connected
					if (!m_connectedPorts.at(i))
					{
						// New connection.
						m_connectedPorts.at(i) = true;
						m_inputSystem->AddInputDevice(InputDeviceTypes::Controller, i);
					}
				}
				else
				{
					// Controller is not connected
					if (m_connectedPorts.at(i))
					{
						// Lost connection.
						m_connectedPorts.at(i) = false;
						m_inputSystem->RemoveInputDevice(InputDeviceTypes::Controller, i);
					}
				}
			}
		}
	}
}
#endif