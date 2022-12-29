#pragma once
#if defined(IS_PLATFORM_WINDOWS) && defined(IS_CPP_WINRT)

#include "Core/TypeAlias.h"
#include "Input/GenericInput.h"

#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.gaming.input.h>

#include <array>

namespace Insight
{
	namespace Input
	{
		class WindowsGamingManager : public InputManager
		{
		public:
			WindowsGamingManager();
			~WindowsGamingManager();

			virtual void Initialise(InputSystem* inputSystem) override;
			virtual void Shutdown() override;
			virtual void Update(float const deltaTime) override;

		private:
			void OnRawControllerAdded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller);
			void OnRawControllerRemoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Gaming::Input::RawGameController const& controller);

			void ExtractDeviceInfo(u32 const controllerIndex);
			void ProcessInput(u32 const controllerIndex);
			void ProcessVibration(u32 const controllerIndex);

		private:
			InputSystem* m_inputSystem = nullptr;

			winrt::event_token m_onControllerAddedEvent = { 0 };
			winrt::event_token m_onControllerRemovedEvent = { 0 };
			std::array<winrt::Windows::Gaming::Input::RawGameController, 4> m_connectControllers = { nullptr, nullptr, nullptr, nullptr };
		};
	}
}

#endif // #if defined(IS_PLATFORM_WINDOWS) && defined(IS_CPP_WINRT)
