#include "App/Engine.h"

#include "Core/Profiler.h"

#include "doctest.h"

namespace Insight
{
	namespace App
	{
//#define WAIT_FOR_PROFILE_CONNECTION

		bool Engine::Init()
		{
#define RETURN_IF_FALSE(x) if (!x) { return false; }
			
			RETURN_IF_FALSE(Graphics::Window::Instance().Init());
			RETURN_IF_FALSE(m_graphicsManager.Init());

			OnInit();

			return true;
		}

		void Engine::Update()
		{
#ifdef WAIT_FOR_PROFILE_CONNECTION
			while (!Optick::IsActive()) { }
#endif
			while (!Graphics::Window::Instance().ShouldClose() && !m_shouldClose)
			{
				IS_PROFILE_FRAME("MainThread");
				m_graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();
			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
		}

#ifdef TESTING
		TEST_SUITE("App Run")
		{
			Graphics::GraphicsManager graphicsManager;
			TEST_CASE("Init")
			{
				CHECK(Graphics::Window::Instance().Init() == true);
				CHECK(graphicsManager.Init());
			}

			TEST_CASE("Update")
			{
				graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
			}

			TEST_CASE("Destroy")
			{
				graphicsManager.Destroy();
				Graphics::Window::Instance().Destroy();
			}
		}
#endif
	}
}