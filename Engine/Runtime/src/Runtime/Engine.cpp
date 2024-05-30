#include "Runtime/Engine.h"

#include "CoreModule.h"
#include "MathsModule.h"
#include "PhysicsModule.h"
#include "GraphicsModule.h"
#include "InputModule.h"
#include "RuntimeModule.h"

#include "Runtime/CommandLineDefines.h"

#include "Core/ImGuiSystem.h"
#include "Core/Profiler.h"
#include "Core/Logger.h"
#include "Core/Timer.h"
#include "Core/Delegate.h"
#include "Core/EnginePaths.h"

#include "Core/Collections/Span.h"

#include "Event/EventSystem.h"

#include "ECS/ECSWorld.h"
#include "ECS/Components/TagComponent.h"

#include "Serialisation/Archive.h"

#include "Physics/PhysicsWorld.h"

#include "imgui.h"

namespace Insight
{
	namespace App
	{
		Core::Timer Engine::s_FrameTimer;

		Engine::Engine()
		{
		}

		Engine::~Engine()
		{

		}

		bool Engine::Init(int argc, char** argv)
		{
			IS_PROFILE_FUNCTION();

			// Systems
			m_systemRegistry.RegisterSystem(&m_assetRegistry);

			m_systemRegistry.RegisterSystem(&m_animationSystem);
			m_systemRegistry.RegisterSystem(&m_audioSystem);
			m_systemRegistry.RegisterSystem(&m_taskSystem);
			m_systemRegistry.RegisterSystem(&m_eventSystem);
			m_systemRegistry.RegisterSystem(&m_inputSystem);
			m_systemRegistry.RegisterSystem(&m_graphicsSystem);
			m_systemRegistry.RegisterSystem(&m_imguiSystem);
			m_systemRegistry.RegisterSystem(&m_worldSystem);
			m_systemRegistry.RegisterSystem(&m_projectSystem);

			m_imguiSystem.Initialise();
			ImGui::SetCurrentContext(m_imguiSystem.GetCurrentContext());

			// Initialise all other "modules" (DLLs)
			CoreModule::Initialise();
			MathsModule::Initialise();
			PhysicsModule::Initialise(&m_imguiSystem);
			GraphicsModule::Initialise(&m_imguiSystem);
			InputModule::Initialise(&m_imguiSystem);
			RuntimeModule::Initialise(&m_imguiSystem);

			IS_LOG_CORE_INFO("Runtime Version {}.{}.{}.",
			ENGINE_VERSION_MAJOIR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);

			const std::string cmdLinePath = "./cmdline.txt";
			Core::CommandLineArgs::ParseCommandLine(argc, argv);
			Core::CommandLineArgs::ParseCommandLine(cmdLinePath.c_str());

			if (Core::CommandLineArgs::GetCommandLineValue(CMD_WAIT_FOR_PROFILER)->GetBool())
			{
				Core::WaitForProfiler();
			}
			if (Core::CommandLineArgs::GetCommandLineValue(CMD_WAIT_FOR_DEBUGGER)->GetBool())
			{
				while (!Platform::IsDebuggerAttached());
			}

			m_assetRegistry.Initialise();

			m_updateThread = std::this_thread::get_id();
			Platform::Initialise();
			EnginePaths::Initialise();

			OnPreInit();

			m_animationSystem.Initialise();
			m_audioSystem.Initialise();
			m_taskSystem.Initialise();
			m_eventSystem.Initialise();

			m_inputSystem.Initialise();
			m_graphicsSystem.Initialise(&m_inputSystem);
			Physics::PhysicsWorld::Initialise();
			m_worldSystem.Initialise();

			m_projectSystem.Initialise();

			OnInit();
			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Initialised);

			if (std::string projectPath = Core::CommandLineArgs::GetCommandLineValue(CMD_PROJECT_PATH)->GetString();
				!projectPath.empty())
			{
				m_projectSystem.OpenProject(projectPath);
			}

			ImGui::GetIO().ConfigInputTrickleEventQueue = false;

			OnPostInit();

			return true;
		}

		void Engine::Update()
		{
			s_FrameTimer.Start();

			while (!Graphics::Window::Instance().ShouldClose() && !m_shouldClose)
			{
				IS_PROFILE_FRAME("MainThread");
				IS_PROFILE_SCOPE("Frame");

				ASSERT(Platform::IsMainThread());

				s_FrameTimer.Stop();
				float delta_time = s_FrameTimer.GetElapsedTimeMillFloat();
				delta_time = std::max(delta_time, 1.0f / 1000.0f);
				s_FrameTimer.Start();

				{
					IS_PROFILE_SCOPE("Game Update");

					Graphics::Window::Instance().Update();
					Graphics::RenderContext::Instance().ImGuiBeginFrame();
					GPUProfiler::Instance().GetFrameData().Draw();

					{
						IS_PROFILE_SCOPE("EventSystem");
						m_eventSystem.Update();
					}

					{
						IS_PROFILE_SCOPE("GraphicsSystem Update");
						m_graphicsSystem.Update();
					}

					{
						IS_PROFILE_SCOPE("AnimationSystem Update");
						m_animationSystem.Update(delta_time);
					}

					{
						IS_PROFILE_SCOPE("InputSsytem Update");
						m_inputSystem.Update(delta_time);
					}

					OnUpdate();

					{
						IS_PROFILE_SCOPE("PhysicsWorld Update");
						TObjectPtr<Runtime::World> activeWorld = m_worldSystem.GetActiveWorld();
						if (activeWorld && activeWorld->GetWorldState() == Runtime::WorldStates::Running)
						{
							Physics::PhysicsWorld::Update(delta_time);
						}
					}

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
					m_graphicsSystem.CreateRenderFrame();
					Graphics::RenderStats::Instance().Draw();
					OnRender();
					{
						IS_PROFILE_SCOPE("GraphicsSystem Render");
						m_graphicsSystem.Render();
					}
				}

				m_inputSystem.ClearFrame();
				++FrameCount;
			}
		}

		void Engine::Destroy()
		{
			IS_PROFILE_FUNCTION();

			Graphics::RenderContext::Instance().WaitForRenderThread();
			Graphics::RenderContext::Instance().GpuWaitForIdle();

			OnDestroy();

			m_eventSystem.Shutdown();

			m_projectSystem.Shutdown();
			
			m_taskSystem.Shutdown();

			m_worldSystem.Shutdown();

			Physics::PhysicsWorld::Shutdown();

			m_audioSystem.Shutdown();

			m_animationSystem.Shutdown();

			m_assetRegistry.Shutdown();

			m_graphicsSystem.Shutdown();

			m_imguiSystem.Shutdown();

			m_inputSystem.Shutdown();

			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Not_Initialised);

			m_systemRegistry.UnregisterSystem(&m_worldSystem);
			m_systemRegistry.UnregisterSystem(&m_inputSystem);
			m_systemRegistry.UnregisterSystem(&m_imguiSystem);
			m_systemRegistry.UnregisterSystem(&m_graphicsSystem);
			m_systemRegistry.UnregisterSystem(&m_projectSystem);
			m_systemRegistry.UnregisterSystem(&m_eventSystem);
			m_systemRegistry.UnregisterSystem(&m_taskSystem);
			m_systemRegistry.UnregisterSystem(&m_audioSystem);
			m_systemRegistry.UnregisterSystem(&m_animationSystem);
			m_systemRegistry.UnregisterSystem(&m_assetRegistry);

			ASSERT(m_systemRegistry.IsEmpty());

			Platform::Shutdown();
		}

		std::string Engine::EngineVersionToString()
		{
			static std::string versionString;
			if (!versionString.empty())
			{
				return versionString;
			}

			versionString += std::to_string(ENGINE_VERSION_MAJOIR);
			versionString += ".";
			versionString += std::to_string(ENGINE_VERSION_MINOR);
			versionString += ".";
			versionString += std::to_string(ENGINE_VERSION_PATCH);
			return versionString;
		}

		bool Engine::IsUpdateThread()
		{
			return Engine::Instance().m_updateThread == std::this_thread::get_id();
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