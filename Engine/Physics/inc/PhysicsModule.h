#pragma once

#include "Physics/Defines.h"

namespace Insight
{
	namespace Core
	{
		class ImGuiSystem;
	}

	class IS_PHYSICS PhysicsModule
	{
	public:
		static void Initialise(Core::ImGuiSystem* imguiSystem);
	};
}