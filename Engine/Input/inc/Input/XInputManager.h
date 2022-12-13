#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/TypeAlias.h"

#include <array>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

namespace Insight
{
	namespace Input
	{
		class InputSystem;

		class XInputManager
		{
		public:
			XInputManager() = default;;
			~XInputManager() = default;

			void Initialise(InputSystem* inputSystem);
			void Shutdown();

			void Update();

		private:
			void ProcessInput(u32 controllerIndex, XINPUT_STATE state);

		private:
			InputSystem* m_inputSystem = nullptr;
			std::array<bool, 4> m_connectedPorts;
		};
	}
}
#endif