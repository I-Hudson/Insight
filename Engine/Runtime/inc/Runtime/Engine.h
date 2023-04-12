#pragma once

#include "Defines.h"
#include "Core/Timer.h"

#include "Core/CommandLineArgs.h"

#include "Core/SystemRegistry.h"
#include "Threading/TaskSystem.h"
#include "Event/EventSystem.h"
#include "Resource/ResourceSystem.h"
#include "Graphics/GraphicsSystem.h"
#include "Input/InputSystem.h"
#include "Core/ImGuiSystem.h"
#include "World/WorldSystem.h"
#include "Runtime/ProjectSystem.h"

#include "Renderpass.h"

constexpr const uint32_t major_bitshift = 22u;
constexpr const uint32_t minor_bitshift = 12u;
constexpr const uint32_t patch_bitshift = 0u;

#define MAKE_ENGINE_VERSION(majoir, minor, patch) (((uint32_t)majoir) << major_bitshift) | (((uint32_t)minor) << minor_bitshift) | (((uint32_t)patch) << patch_bitshift)
#define ENGINE_VERSION MAKE_ENGINE_VERSION(1, 0, 0)

#define ENGINE_VERSION_MAJOIR (((uint32_t)ENGINE_VERSION) >> 22u)
#define ENGINE_VERSION_MINOR  (((uint32_t)ENGINE_VERSION) >> 12u) & 0xFF
#define ENGINE_VERSION_PATCH  (((uint32_t)ENGINE_VERSION)) & 0xFF

namespace Insight
{
	namespace App
	{
		/*
			Main engine class.
		*/
		class IS_RUNTIME Engine : public Core::Singleton<Engine>
		{
		public:

			bool Init(int argc, char** argv);
			void Update();
			void Destroy();

			void ForceClose() { m_shouldClose = true; }

			virtual void OnInit() { }
			virtual void OnUpdate() { }
			virtual void OnDestroy() { }

			Core::SystemRegistry& GetSystemRegistry() { return m_systemRegistry; }

			static bool IsUpdateThread();

			static Core::Timer s_FrameTimer;
		private:
			Graphics::Renderpass m_renderpasses;

			bool m_shouldClose = false;

			std::thread::id m_updateThread;

			// Systems
			Core::SystemRegistry m_systemRegistry;
			Threading::TaskSystem m_taskSystem;
			Core::EventSystem m_eventSystem;
			Runtime::ResourceSystem m_resourceSystem;
			Runtime::GraphicsSystem m_graphicsSystem;
			Core::ImGuiSystem m_imguiSystem;
			Input::InputSystem m_inputSystem;
			Runtime::WorldSystem m_worldSystem;
			Runtime::ProjectSystem m_projectSystem;
		};
	}
}