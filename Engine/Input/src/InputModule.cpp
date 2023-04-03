#include "InputModule.h"
#include "Core/ImGuiSystem.h"

#include <imgui.h>

namespace Insight
{
	void InputModule::Initialise(Core::ImGuiSystem* imguiSystem)
	{
		SET_IMGUI_CURRENT_CONTEXT();
	}
}