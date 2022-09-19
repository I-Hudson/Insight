#pragma once

#include "Core/Defines.h"

struct ImGuiContext;

namespace Insight
{
	namespace Core
	{
		class IS_CORE ImGuiSystem
		{
		public:
			static void Init();
			static void Shutdown();
			static ImGuiContext* GetCurrentContext();
		};
	}
}