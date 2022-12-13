#pragma once

#include "Input/Defines.h"

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class IS_INPUT InputModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}