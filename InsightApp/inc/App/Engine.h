#pragma once

#include "Defines.h"

// -- Managers --
#include "Graphics/GraphicsManager.h"

#include "Graphics/Window.h"

//#define TESTING

namespace Insight
{
	namespace App
	{
		/*
			Main engine class.
		*/
		class IS_APP Engine
		{
		public:

			bool Init();
			void Update();
			void Destroy();

			void ForceClose() { m_shouldClose = true; }

			virtual void OnInit() { }
			virtual void OnDestroy() { }

		private:
			Graphics::GraphicsManager m_graphicsManager;
			bool m_shouldClose = false;
		};
	}
}