#include "App/Engine.h"

#include "Core/Profiler.h"
#include "Core/Timer.h"

#include "Event/EventManager.h"
#include "Input/InputManager.h"

#include "ECS/ECSWorld.h"
#include "ECS/Components/TagComponent.h"

#include "imgui.h"

namespace Insight
{
	namespace App
	{
///#define WAIT_FOR_PROFILE_CONNECTION

		Core::Timer Engine::s_FrameTimer;

		bool Engine::Init()
		{
#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			m_eventManager = MakeUPtr<Core::EventManager>();

			RETURN_IF_FALSE(Graphics::Window::Instance().Init());
			RETURN_IF_FALSE(m_graphicsManager.Init());
			m_renderpasses.Create();

			RETURN_IF_FALSE(Input::InputManager::InitWithWindow(&Graphics::Window::Instance()));

			ImGuiContext* context = nullptr;
			ImGuiIO* io = nullptr;
			Graphics::GraphicsManager::Instance().SetImGuiContext(&context, &io);
			ImGui::SetCurrentContext(context);
			ImGui::GetIO() = *io;

			m_sceneManager = MakeUPtr<SceneManager>();

			WPtr<Scene> newScene = m_sceneManager->CreateScene("New Scene");
			SceneManager::Instance().SetActiveScene(newScene);

			Ptr<ECS::ECSWorld> ecs_world = newScene.Lock()->GetECSWorld();
			ECS::Entity* e = ecs_world->AddEntity("Test_Entity");
			ECS::TagComponent* tag_component = static_cast<ECS::TagComponent*>(e->GetComponentByName(ECS::TagComponent::Type_Name));
			tag_component->AddTag("Test_Tag");

			Ptr<ECS::Entity> test_entity = ecs_world->GetEntityByName("Test_Entity");
			auto all_tags = static_cast<ECS::TagComponent*>(e->GetComponentByName(ECS::TagComponent::Type_Name))->GetAllTags();

			OnInit();

			return true;
		}

		void Engine::Update()
		{
#ifdef WAIT_FOR_PROFILE_CONNECTION
			Core::WaitForProfiler();
#endif
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
			m_sceneManager.Reset();
			m_renderpasses.Destroy();
			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
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