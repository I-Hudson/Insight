#include "GraphicsModule.h"
#include "Core/ImGuiSystem.h"

#include <imgui.h>

namespace Insight
{
	void GraphicsModule::Initialise(Core::ImGuiSystem* imguiSystem)
	{
		SET_IMGUI_CURRENT_CONTEXT();
	}
}