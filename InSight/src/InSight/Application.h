#pragma once

#include "Core.h"

namespace Insight
{
	class IS_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	};

	Application* CreateApplication();
}
