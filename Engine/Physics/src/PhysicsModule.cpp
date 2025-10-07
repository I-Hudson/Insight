#include "PhysicsModule.h"
#include "Core/ImGuiSystem.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

namespace Insight
{
    void PhysicsModule::Initialise(Core::ImGuiSystem* imguiSystem)
    {
#ifndef IS_MONOLITH
        ImGui::SetCurrentContext(imguiSystem->GetCurrentContext()); 
        ImGuiMemAllocFunc allocFunc; 
        ImGuiMemFreeFunc freeFunc; 
        void* pUsedData; 
        imguiSystem->GetAllocatorFunctions(allocFunc, freeFunc, pUsedData); 
        ImGui::SetAllocatorFunctions(allocFunc, freeFunc, pUsedData);;
        SET_SPDLOG_LOGGERS();
#endif
    }

    void PhysicsModule::Shutdown(Core::ImGuiSystem* imguiSystem)
    {
        ASSERT(ImGui::GetCurrentContext() == imguiSystem->GetCurrentContext());
    }
}