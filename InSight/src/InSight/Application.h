#pragma once

#include "Core.h"
#include <memory>

namespace Insight
{

	namespace Module
	{
		class ModuleManager;
		class WindowModule;
		class GraphicsModule;
		class InputModule;
	}

	class Camera;

	namespace Memory
	{
		class MemoryManager;
	}

	class IS_API Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Create() = 0;
		virtual void Update(const float deltaTime) = 0;
		virtual void Draw() = 0;

		void Run();

	private:

		Memory::MemoryManager* m_memoryManager;

		Module::ModuleManager* m_moduleManager;
		Module::WindowModule* m_windowModule;
		Module::GraphicsModule* m_graphicsModule;
		Module::InputModule* m_inputModule;

		UniquePtr<Camera> m_mainCamera;
	};

	Application* CreateApplication();
}
