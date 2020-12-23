#pragma once

#include "Core.h"
#include <memory>
#include <thread>

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

	namespace Memory
	{
		class MemoryManager;
	}

	enum UpdateThreadState
	{
		SAME_FRMAE = 0,
		ONE_FRAME_AHEAD,
		TWO_FRAME_AHEAD
	};

	class IS_API Application
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

	protected:

		std::mutex m_mutex;
		std::thread m_updateThread;
		std::thread m_renderThread;

		UpdateThreadState m_updateThreadState;

		bool m_isRunning = true;
		bool m_triggerRender = true;
		bool m_renderComplete = true;

		Memory::MemoryManager* m_memoryManager;

		Module::ModuleManager* m_moduleManager;
		SharedPtr<Module::WindowModule> m_windowModule;
		SharedPtr<Module::GraphicsModule> m_graphicsModule;
		SharedPtr<Module::InputModule> m_inputModule;
	};

	Application* CreateApplication();
}
