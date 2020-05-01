#pragma once

#include "Core.h"

namespace Insight
{

	namespace Module
	{
		class ModuleManager;
		class WindowModule;
		class GraphicsModule;
		class InputModule;
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

		virtual void Update(const float deltaTime) = 0;
		virtual void Draw() = 0;

		void Run();

	private:

		Memory::MemoryManager* m_memoryManager;

		Module::ModuleManager* m_moduleManager;
		Module::WindowModule* m_windowModule;
		Module::GraphicsModule* m_graphicsModule;
		Module::InputModule* m_inputModule;
	};

	Application* CreateApplication();
}
