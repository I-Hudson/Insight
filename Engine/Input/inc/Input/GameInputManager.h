#pragma once
#if defined(IS_PLATFORM_WINDOWS) && defined(IS_GAME_INPUT)

#include "Core/TypeAlias.h"
#include "Input/GenericInput.h"

#include "GameInput.h"
#define GameInputNS GameInput::v3

#include <vector>

namespace Insight
{
	namespace Input
	{
		class GameInputManager : public InputManager
		{
		public:
			GameInputManager();
			virtual ~GameInputManager() override;

			virtual bool Initialise(InputSystem* inputSystem) override;
			virtual void Shutdown() override;
			virtual void Update(float const deltaTime) override;

		private:
			static void OnDeviceCallback(GameInputNS::GameInputCallbackToken callbackToken, void* context, GameInputNS::IGameInputDevice* device, u64 timestamp,
				GameInputNS::GameInputDeviceStatus currentStatus, GameInputNS::GameInputDeviceStatus previousStatus);

			void ExtractDeviceInfo(u32 const controllerIndex);
			void ProcessInput(u32 const controllerIndex);
			void ProcessVibration(u32 const controllerIndex);

		private:
			InputSystem* m_inputSystem = nullptr;

			GameInputNS::IGameInput* m_gameInput = nullptr;
			GameInputNS::GameInputCallbackToken m_onDeviceCallbackToken = 0;

			std::vector<GameInputNS::IGameInputDevice*> m_connectedDevices;
		};
	}
}

#endif // #if defined(IS_PLATFORM_WINDOWS) && defined(IS_GAME_INPUT)
