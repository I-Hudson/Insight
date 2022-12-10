#include "Runtime/Engine.h"
#include "Runtime/CommandLineDefines.h"

#include "Core/ImGuiSystem.h"
#include "Core/Profiler.h"
#include "Core/Logger.h"
#include "Core/Timer.h"
#include "Core/Delegate.h"

#include "Event/EventManager.h"
#include "Input/InputManager.h"

#include "ECS/ECSWorld.h"
#include "ECS/Components/TagComponent.h"

#include "Graphics/Mesh.h"

#include "imgui.h"

namespace Insight
{
	namespace App
	{
		Core::Timer Engine::s_FrameTimer;

		auto square(int x) -> int { return x * x; }
		auto divide(int x) -> int { return x / x; }

		bool Engine::Init(int argc, char** argv)
		{
			Platform::Initialise();

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

			m_taskManger.Init();

			auto testDelegate = Core::Action<int(int)>();
			testDelegate.Bind<&square>();
			testDelegate.Bind<&divide>();
			assert(testDelegate(2) == 4);
			testDelegate.Unbind<&square>();
			testDelegate.Bind([](int x) -> int
				{
					return x * x;
				});
			assert(testDelegate(5) == 25);

			auto str = std::string{ "Hello" };
			auto testDelegateString = Core::Action<std::string::size_type()>{};
			testDelegateString.Bind<&std::string::size>(&str);
			assert(testDelegateString() == str.size());
			testDelegateString.Unbind<&std::string::size>(&str);

			//Threading::TaskSharedPtr task100 = Threading::TaskManager::CreateTask([]()
			//	{
			//		IS_PROFILE_SCOPE("Task100");
			//		std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 20));
			//		IS_CORE_INFO("Thread sleep for 100 ms.");
			//	});

			Threading::TaskManager::CreateTask([]()
				{
					IS_PROFILE_SCOPE("Task20");
					std::this_thread::sleep_for(std::chrono::milliseconds(5000));
					IS_CORE_INFO("Thread sleep for 20 ms.");
					return 45;
				});

#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			Core::ImGuiSystem::Init();
			ImGui::SetCurrentContext(Core::ImGuiSystem::GetCurrentContext());

			m_eventManager = MakeUPtr<Core::EventManager>();

			RETURN_IF_FALSE(Graphics::Window::Instance().Init());
			RETURN_IF_FALSE(m_graphicsManager.Init());

			RETURN_IF_FALSE(Input::InputManager::InitWithWindow(&Graphics::Window::Instance()));

			m_sceneManager = MakeUPtr<SceneManager>();
			WPtr<Scene> newScene = m_sceneManager->CreateScene("New Scene");
			SceneManager::Instance().AddActiveScene(newScene);

			m_renderpasses.Create();

//#define SCENE_TEST
#ifdef SCENE_TEST
			Ptr<ECS::ECSWorld> ecs_world = newScene.Lock()->GetECSWorld();
			ECS::Entity* e = ecs_world->AddEntity("Test_Entity");
			ECS::TagComponent* tag_component = static_cast<ECS::TagComponent*>(e->GetComponentByName(ECS::TagComponent::Type_Name));
			tag_component->AddTag("Test_Tag");

			Ptr<ECS::Entity> test_entity = ecs_world->GetEntityByName("Test_Entity");
			auto all_tags = static_cast<ECS::TagComponent*>(e->GetComponentByName(ECS::TagComponent::Type_Name))->GetAllTags();
#endif
#undef SCENE_TEST

			OnInit();

			return true;
		}

		void Engine::Update()
		{
			s_FrameTimer.Start();

			while (!Graphics::Window::Instance().ShouldClose() && !m_shouldClose)
			{
				IS_PROFILE_FRAME("MainThread");
				s_FrameTimer.Stop();
				float delta_time = s_FrameTimer.GetElapsedTimeMillFloat();
				s_FrameTimer.Start();

				{
					IS_PROFILE_SCOPE("Game Update");
					m_eventManager->Update();

					OnUpdate();

					m_sceneManager->EarlyUpdate();
					m_sceneManager->Update(delta_time);
					m_sceneManager->LateUpdate();
				}

				m_renderpasses.Render();
				m_graphicsManager.Update(0.0f);
				Input::InputManager::Update();
				Graphics::Window::Instance().Update();
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();
			
			m_renderpasses.Destroy();
			m_sceneManager.Reset();
			m_resource_manager.UnloadAll();
			m_eventManager.Reset();

			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
			Core::ImGuiSystem::Shutdown();

			m_taskManger.Destroy();
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