#include "PhysicsModule.h"
#include "Core/ImGuiSystem.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

namespace Insight
{
    void PhysicsModule::Initialise(Core::ImGuiSystem* imguiSystem)
    {
#ifndef IS_MONOLITH
        SET_IMGUI_CURRENT_CONTEXT();
        SET_SPDLOG_LOGGERS();
#endif
    }

    void PhysicsModule::Shutdown()
    {
        ImGui::SetCurrentContext(nullptr);
    }
}