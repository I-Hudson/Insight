#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include <array>

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
			InputSystem* m_inputSystem = nullptr;
			std::array<bool, 4> m_connectedPorts;
		};
	}
}
#endif