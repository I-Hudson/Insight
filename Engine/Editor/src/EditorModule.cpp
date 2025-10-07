#include "EditorModule.h"
#include "Core/ImGuiSystem.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

namespace Insight
{
    void EditorModule::Initialise(Core::ImGuiSystem* imguiSystem)
    {
		SET_IMGUI_CURRENT_CONTEXT();
        SET_SPDLOG_LOGGERS();
    }

    void EditorModule::Shutdown()
    {
        ImGui::SetCurrentContext(nullptr);
    }
}