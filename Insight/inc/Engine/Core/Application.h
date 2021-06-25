#pragma once

#include "Common.h"
#include "Engine/Templates/TSingleton.h"
#include <memory>
#include <thread>
#include <mutex>

class Scene;
namespace Insight
{
	namespace Module
	{
		class ModuleManager;
		class WindowModule;
		class GraphicsModule;
		class InputModule;
		class AssetModule;
	}

	enum UpdateThreadState
	{
		SAME_FRMAE = 0,
		ONE_FRAME_AHEAD,
		TWO_FRAME_AHEAD
	};

	enum ApplicationState
	{
		Init,
		Loading,
		Running,
		Unloading,
		Deinit
	};

	class Application : Core::TSingleton<Application>
	{
	public:
		Application();
		virtual ~Application();

		virtual void Create() = 0;
		virtual void Update(const float deltaTime) = 0;
		virtual void Draw() = 0;
		virtual void OnFrameEnd() { }

		void Run();
		void RenderLoop();

		static uint64_t FrameCount;
		static ApplicationState GetState()
		{
			return Application::Instance()->m_state;
		}

	protected:
		std::mutex m_mutex;
		std::thread m_updateThread;
		std::thread m_renderThread;
		ApplicationState m_state;

		UpdateThreadState m_updateThreadState;

		bool m_isRunning = true;
		bool m_triggerRender = true;
		bool m_renderComplete = true;
		Scene* m_utitledScene;

		Module::ModuleManager* m_moduleManager;
		Module::WindowModule* m_windowModule;
		Module::GraphicsModule* m_graphicsModule;
		Module::InputModule* m_inputModule;
	};
}
Insight::Application* CreateApplication();