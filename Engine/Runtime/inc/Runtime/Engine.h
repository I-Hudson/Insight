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

#include "Renderpass.h"


///#define TESTING

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

			static Core::Timer s_FrameTimer;
		private:
			Graphics::Renderpass m_renderpasses;

			bool m_shouldClose = false;

			// Systems
			Core::SystemRegistry m_systemRegistry;
			Threading::TaskSystem m_taskSystem;
			Core::EventSystem m_eventSystem;
			Runtime::ResourceSystem m_resourceSystem;
			Runtime::GraphicsSystem m_graphicsSystem;
			Core::ImGuiSystem m_imguiSystem;
			Input::InputSystem m_inputSystem;
			Runtime::WorldSystem m_worldSystem;
		};
	}
}