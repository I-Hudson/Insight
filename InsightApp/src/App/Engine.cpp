#include "App/Engine.h"
#include "Tracy.hpp"

namespace Insight
{
	namespace App
	{
//#define TRACY_WAIT_FOR_CONNECTION

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
#ifdef TRACY_WAIT_FOR_CONNECTION
			while (!tracy::GetProfiler().IsConnected()) { }
#endif
			while (!Graphics::Window::Instance().ShouldClose())
			{
				m_graphicsManager.Update(0.0f);
				Graphics::Window::Instance().Update();
				
				FrameMark;
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();

			m_graphicsManager.Destroy();
			Graphics::Window::Instance().Destroy();
		}
	}
}