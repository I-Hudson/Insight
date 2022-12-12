#pragma once

#include "Graphics/Defines.h"

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class IS_GRAPHICS GraphicsModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}