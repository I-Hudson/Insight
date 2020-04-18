#pragma once

#include "Core.h"

namespace Insight
{
	namespace Module
	{
		class ModuleManager;
	}

	class IS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:


		class Module::ModuleManager* m_moduleManager;

	};

	Application* CreateApplication();
}
