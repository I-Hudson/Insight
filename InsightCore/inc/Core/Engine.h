#pragma once

#include "Defines.h"

// -- Managers --
#include "Graphics/GraphicsManager.h"

namespace Insight
{
	namespace Core
	{
		/*
			Main engine class.
		*/
		class IS_CORE Engine
		{
		public:

			void Init();
			void Update();
			void Destroy();

			virtual void OnInit() { }
			virtual void OnDestroy() { }

		private:

			Graphics::GraphicsManager m_graphicsManager;
		};
	}
}