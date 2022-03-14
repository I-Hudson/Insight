#pragma once

#include "Defines.h"

// -- Managers --
#include "Graphics/GraphicsManager.h"

#include "Graphics/Window.h"

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

			virtual void OnInit() { }
			virtual void OnDestroy() { }

		private:
			Graphics::GraphicsManager m_graphicsManager;
		};
	}
}