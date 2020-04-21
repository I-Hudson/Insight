#pragma once

#include "Core.h"

namespace Insight
{

	namespace Module
	{
		class ModuleManager;
		class WindowModule;
		class GraphicsModule;
	}

	namespace Memory
	{
		class MemoryManager;
	}

	class IS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:

		Memory::MemoryManager* m_memoryManager;

		Module::ModuleManager* m_moduleManager;
		Module::WindowModule* m_windowModule;
		Module::GraphicsModule* m_graphicsModule;
	};

	Application* CreateApplication();
}
