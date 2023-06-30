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
#include "Core/EnginePaths.h"

#include "Core/Collections/Span.h"

#include "Event/EventSystem.h"

#include "ECS/ECSWorld.h"
#include "ECS/Components/TagComponent.h"

#include "Graphics/SimplygonWrapper.h"
#include "Resource/ResourcePack.h"
#include "Resource/ResourceManager.h"

#include "Serialisation/Archive.h"

#include "SplashScreen.h"

#include "imgui.h"

namespace Insight
{
	namespace App
	{
		Core::Timer Engine::s_FrameTimer;

		SplashScreen splashScreen;

		bool Engine::Init(int argc, char** argv)
		{
			IS_CORE_INFO("Runtime Version {}.{}.{}.", 
				ENGINE_VERSION_MAJOIR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);

			m_updateThread = std::this_thread::get_id();

			// Systems
			m_systemRegistry.RegisterSystem(&m_taskSystem);
			m_systemRegistry.RegisterSystem(&m_eventSystem);
			m_systemRegistry.RegisterSystem(&m_resourceSystem);
			m_systemRegistry.RegisterSystem(&m_inputSystem);
			m_systemRegistry.RegisterSystem(&m_graphicsSystem);
			m_systemRegistry.RegisterSystem(&m_imguiSystem);
			m_systemRegistry.RegisterSystem(&m_worldSystem);
			m_systemRegistry.RegisterSystem(&m_projectSystem);

			Platform::Initialise();
			EnginePaths::Initialise();

			static const std::string splashScreenBackGroundPath = EnginePaths::GetResourcePath() + "/Insight/cover.png";
			splashScreen.Init(860, 420);
			splashScreen.SetBackgroundImage(splashScreenBackGroundPath.c_str());
			splashScreen.Show();

			const std::string cmdLinePath = EnginePaths::GetExecutablePath() + "/cmdline.txt";
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

			m_inputSystem.Initialise();
			m_graphicsSystem.Initialise(&m_inputSystem);
			m_worldSystem.Initialise();

			m_projectSystem.SetResourceSystem(&m_resourceSystem);
			m_projectSystem.Initialise();

			OnInit();
			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Initialised);

			if (std::string projectPath = Core::CommandLineArgs::GetCommandLineValue(CMD_PROJECT_PATH)->GetString();
				!projectPath.empty())
			{
				m_projectSystem.OpenProject(projectPath);
			}

			splashScreen.Destroy();

			ImGui::GetIO().ConfigInputTrickleEventQueue = false;

			Runtime::ResourcePack resoucePack("TestPack.isResourcePack");

			auto resrouceId = Runtime::ResourceId("E://User//Documents//SourceControl//Github//C++Porjects//Insight//DemoProject//Content//Textures//Christmas_Cute_Roadhog.png"
				, Runtime::Texture2D::GetStaticResourceTypeId());
			resoucePack.AddResource(Runtime::ResourceManager::LoadSync(resrouceId).Get());

			resoucePack.Save();

			Runtime::ResourcePack* loadResoucePack = (Runtime::ResourcePack*)Runtime::ResourceManager::LoadSync(
				Runtime::ResourceId("TestPack.isResourcePack", Runtime::ResourcePack::GetStaticResourceTypeId())).Get();


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
						IS_PROFILE_SCOPE("InputSsytem Update");
						m_inputSystem.Update(delta_time);
					}

					{
						IS_PROFILE_SCOPE("InputSystem Update");
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
			Graphics::RenderContext::Instance().WaitForRenderThread();
			Graphics::RenderContext::Instance().GpuWaitForIdle();

			OnDestroy();
			
			m_worldSystem.Shutdown();

			m_resourceSystem.Shutdown();

			m_graphicsSystem.Shutdown();

			m_imguiSystem.Shutdown();

			m_inputSystem.Shutdown();
			m_eventSystem.Shutdown();

			m_projectSystem.Shutdown();

			m_taskSystem.Shutdown();

			m_systemRegistry.VerifyAllSystemsStates(Core::SystemStates::Not_Initialised);

			m_systemRegistry.UnregisterSystem(&m_worldSystem);
			m_systemRegistry.UnregisterSystem(&m_inputSystem);
			m_systemRegistry.UnregisterSystem(&m_imguiSystem);
			m_systemRegistry.UnregisterSystem(&m_graphicsSystem);
			m_systemRegistry.UnregisterSystem(&m_resourceSystem);
			m_systemRegistry.UnregisterSystem(&m_projectSystem);
			m_systemRegistry.UnregisterSystem(&m_eventSystem);
			m_systemRegistry.UnregisterSystem(&m_taskSystem);

			ASSERT(m_systemRegistry.IsEmpty());

			SimplygonWrapper::Shutdown();

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