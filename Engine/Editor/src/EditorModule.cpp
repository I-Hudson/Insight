#include "EditorModule.h"
#include "Core/ImGuiSystem.h"

namespace Insight
{
    void EditorModule::Initialise(Core::ImGuiSystem* imguiSystem)
    {
		SET_IMGUI_CURRENT_CONTEXT();
    }
}