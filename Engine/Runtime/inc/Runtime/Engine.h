#pragma once

#include "Defines.h"
#include "Core/Timer.h"

#include "Core/CommandLineArgs.h"

/// -- Managers --
#include "Threading/TaskManager.h"

#include "Graphics/GraphicsManager.h"
#include "Scene/SceneManager.h"

#include "Event/EventManager.h"
#include "Resource/Resource.h"

#include "Renderpass.h"

#include "Graphics/Window.h"


///#define TESTING

namespace Insight
{
	namespace App
	{
		/*
			Main engine class.
		*/
		class IS_RUNTIME Engine
		{
		public:

			bool Init(int argc, char** argv);
			void Update();
			void Destroy();

			void ForceClose() { m_shouldClose = true; }

			virtual void OnInit() { }
			virtual void OnUpdate() { }
			virtual void OnDestroy() { }

			static Core::Timer s_FrameTimer;
		private:
			Graphics::GraphicsManager m_graphicsManager;
			Graphics::Renderpass m_renderpasses;

			bool m_shouldClose = false;

			Threading::TaskManager m_taskManger;

			UPtr<SceneManager> m_sceneManager;
			UPtr<Core::EventManager> m_eventManager;
			Runtime::ResourceManager m_resource_manager;

		};
	}
}