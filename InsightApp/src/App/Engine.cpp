#include "App/Engine.h"

#include "optick.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

namespace Insight
{
	namespace App
	{
//#define WAIT_FOR_PROFILE_CONNECTION

		bool Engine::Init()
		{
#ifdef TESTING
			doctest::Context().run();
			return false;
#endif


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
				OPTICK_FRAME("MainThread");
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
		TEST_CASE("lots of nested subcases")
		{
			Graphics::GraphicsManager graphicsManager;
			SUBCASE("Init")
			{
				CHECK(Graphics::Window::Instance().Init() == true);
				CHECK(graphicsManager.Init());
			}

			SUBCASE("Update")
			{
				graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
			}

			SUBCASE("Destroy")
			{
				graphicsManager.Destroy();
				Graphics::Window::Instance().Destroy();
			}
		}
#endif
	}
}