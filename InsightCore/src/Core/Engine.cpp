#include "Core/Engine.h"

namespace Insight
{
	namespace Core
	{
		void Engine::Init()
		{
			m_graphicsManager.Init();

			OnInit();
		}

		void Engine::Update()
		{
			while (false)
			{

				m_graphicsManager.Update(0.0f);
			}
		}

		void Engine::Destroy()
		{
			OnDestroy();

			m_graphicsManager.Destroy();
		}
	}
}