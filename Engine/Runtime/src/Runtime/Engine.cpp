#include "Runtime/Engine.h"

#include "CoreModule.h"
#include "MathsModule.h"
#include "GraphicsModule.h"
#include "InputModule.h"

#include "Runtime/CommandLineDefines.h"

#include "Core/ImGuiSystem.h"
#include "Core/Profiler.h"
#include "Core/Logger.h"
#include "Core/Timer.h"
#include "Core/Delegate.h"

#include "Core/Collections/Span.h"

#include "Event/EventSystem.h"

#include "ECS/ECSWorld.h"
#include "ECS/Components/TagComponent.h"

#include "Graphics/Mesh.h"

#include "SplashScreen.h"

#include "imgui.h"

namespace Insight
{
	namespace App
	{
		Core::Timer Engine::s_FrameTimer;

		auto square(int x) -> int { return x * x; }
		auto divide(int x) -> int { return x / x; }

		SplashScreen splashScreen;

		bool Engine::Init(int argc, char** argv)
		{
			Platform::Initialise();

			splashScreen.Init(860, 420);
			splashScreen.SetBackgroundImage("./Resources/Insight/cover.png");
			splashScreen.Show();

			Core::CommandLineArgs::ParseCommandLine(argc, argv);
			Core::CommandLineArgs::ParseCommandLine("./cmdline.txt");

			if (Core::CommandLineArgs::GetCommandLineValue(CMD_WAIT_FOR_PROFILER)->GetBool())
			{
				Core::WaitForProfiler();
			}
			if (Core::CommandLineArgs::GetCommandLineValue(CMD_WAIT_FOR_DEBUGGER)->GetBool())
			{
				while (!IsDebuggerPresent())
				{ }
			}

#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			// Systems
			m_systemRegistry.RegisterSystem(&m_taskSystem);
			m_systemRegistry.RegisterSystem(&m_eventSystem);
			m_systemRegistry.RegisterSystem(&m_resourceSystem);
			m_systemRegistry.RegisterSystem(&m_inputSystem);
			m_systemRegistry.RegisterSystem(&m_graphicsSystem);
			m_systemRegistry.RegisterSystem(&m_imguiSystem);
			m_systemRegistry.RegisterSystem(&m_worldSystem);

			m_taskSystem.Initialise();
			m_eventSystem.Initialise();
			m_resourceSystem.Initialise();
			m_imguiSystem.Initialise();
			ImGui::SetCurrentContext(m_imguiSystem.GetCurrentContext());

			// Initialise all other "modules" (DLLs)
			CoreModule::Initialise();
			MathsModule::Initialise();
			GraphicsModule::Initialise(&m_imguiSystem);
			InputModule::Initialise(&m_imguiSystem);

			m_graphicsSystem.Initialise(&m_inputSystem);
			m_inputSystem.Initialise();
			m_worldSystem.Initialise();

			OnInit();
			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Initialised);

			m_renderpasses.Create();
			splashScreen.Destroy();

			std::string str = "Test string for slice.";
			std::string_view view(str);
			Span<char> span(str);

			char c = span[12];
			char& cRef = span[12];

			return true;
		}

		void Engine::Update()
		{
			s_FrameTimer.Start();

			while (!Graphics::Window::Instance().ShouldClose() && !m_shouldClose)
			{
				IS_PROFILE_FRAME("MainThread");
				IS_PROFILE_SCOPE("Frame");

				s_FrameTimer.Stop();
				float delta_time = s_FrameTimer.GetElapsedTimeMillFloat();
				s_FrameTimer.Start();

				{
					IS_PROFILE_SCOPE("Game Update");

					{
						IS_PROFILE_SCOPE("EventSystem");
						m_eventSystem.Update();
					}

					{
						IS_PROFILE_SCOPE("GraphicsSystem Update");
						m_graphicsSystem.Update();
					}

					{
						IS_PROFILE_SCOPE("InputSsytem Update");
						m_inputSystem.Update(delta_time);
					}

					{
						IS_PROFILE_SCOPE("InputSsytem Update");
						m_resourceSystem.Update(delta_time);
					}

					OnUpdate();

					{
						IS_PROFILE_SCOPE("EarlyUpdate");
						m_worldSystem.EarlyUpdate();
					}
					{
						IS_PROFILE_SCOPE("Update");
						m_worldSystem.Update(delta_time);
					}
					{
						IS_PROFILE_SCOPE("LateUpdate");
						m_worldSystem.LateUpdate();
					}
				}

				{
					IS_PROFILE_SCOPE("Render Update");

					m_renderpasses.Render();
					{
						IS_PROFILE_SCOPE("GraphicsSystem Render");
						m_graphicsSystem.Render();
					}
				}

				m_inputSystem.ClearFrame();
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();
			
			m_renderpasses.Destroy();

			m_resourceSystem.Shutdown();

			m_worldSystem.Shutdown();
			m_inputSystem.Shutdown();
			m_graphicsSystem.Shutdown();
			m_imguiSystem.Shutdown();
			m_eventSystem.Shutdown();
			m_taskSystem.Shutdown();

			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Not_Initialised);

			m_systemRegistry.UnregisterSystem(&m_worldSystem);
			m_systemRegistry.UnregisterSystem(&m_inputSystem);
			m_systemRegistry.UnregisterSystem(&m_imguiSystem);
			m_systemRegistry.UnregisterSystem(&m_graphicsSystem);
			m_systemRegistry.UnregisterSystem(&m_resourceSystem);
			m_systemRegistry.UnregisterSystem(&m_eventSystem);
			m_systemRegistry.UnregisterSystem(&m_taskSystem);

			ASSERT(m_systemRegistry.IsEmpty());

			Platform::Shutdown();
		}
	}
}

#ifdef TESTING
#include "doctest.h"
		TEST_SUITE("App Run")
		{
			///using namespace Insight;
			///Graphics::GraphicsManager graphicsManager;
			///TEST_CASE("Init")
			///{
			///	CHECK(Graphics::Window::Instance().Init() == true);
			///	CHECK(graphicsManager.Init());
			///}
			///
			///TEST_CASE("Update")
			///{
			///	graphicsManager.Update(0.0f);
			///	Graphics::Window::Instance().Update();
			///}
			///
			///TEST_CASE("Destroy")
			///{
			///	graphicsManager.Destroy();
			///	Graphics::Window::Instance().Destroy();
			///}
		}
#endif