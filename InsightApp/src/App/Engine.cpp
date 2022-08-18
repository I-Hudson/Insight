#include "App/Engine.h"

#include "Core/Profiler.h"
#include "Core/Timer.h"

#include "Event/EventManager.h"

namespace Insight
{
	namespace App
	{
//#define WAIT_FOR_PROFILE_CONNECTION

		bool Engine::Init()
		{
#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			m_eventManager = MakeUPtr<Core::EventManager>();

			RETURN_IF_FALSE(Graphics::Window::Instance().Init());
			RETURN_IF_FALSE(m_graphicsManager.Init());

			m_sceneManager = MakeUPtr<SceneManager>();

			WPtr<Scene> newScene = m_sceneManager->CreateScene("New Scene");
			SceneManager::Instance().SetActiveScene(newScene);

			OnInit();

			return true;
		}

		void Engine::Update()
		{
#ifdef WAIT_FOR_PROFILE_CONNECTION
			Core::WaitForProfiler();
#endif
			Core::Timer frameTimer;
			frameTimer.Start();

			while (!Graphics::Window::Instance().ShouldClose() && !m_shouldClose)
			{
				IS_PROFILE_FRAME("MainThread");
				frameTimer.Stop();
				float deltaTime = static_cast<float>(frameTimer.GetElapsedTimeMill().count() / 1000);
				frameTimer.Start();

				{
					IS_PROFILE_SCOPE("Game Update");
					m_eventManager->Update();
					m_sceneManager->Update(deltaTime);
				}

				m_graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();
			m_sceneManager.Reset();
			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
		}
	}
}

#ifdef TESTING
#include "doctest.h"
		TEST_SUITE("App Run")
		{
			//using namespace Insight;
			//Graphics::GraphicsManager graphicsManager;
			//TEST_CASE("Init")
			//{
			//	CHECK(Graphics::Window::Instance().Init() == true);
			//	CHECK(graphicsManager.Init());
			//}
			//
			//TEST_CASE("Update")
			//{
			//	graphicsManager.Update(0.0f);
			//	Graphics::Window::Instance().Update();
			//}
			//
			//TEST_CASE("Destroy")
			//{
			//	graphicsManager.Destroy();
			//	Graphics::Window::Instance().Destroy();
			//}
		}
#endif