#include "Editor/EditorWindows/SystemInformationWindow.h"

#include "Platforms/Platform.h"
#include "Runtime/Engine.h"

#include "Core/Profiler.h"

#include <imgui.h>

#include <string>

namespace Insight
{
    namespace Editor
    {
        SystemInformationWindow::SystemInformationWindow()
        { }

        SystemInformationWindow::SystemInformationWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        SystemInformationWindow::SystemInformationWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        SystemInformationWindow::~SystemInformationWindow()
        { }


        void SystemInformationWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            Core::CPUInformation cpuInfo = Platform::GetCPUInformation();
            float speedInGHz = static_cast<float>(cpuInfo.SpeedInMHz) / 1000;

            ImGui::Text("CPU Information");
            ImGui::Separator();

            ImGui::Indent(); 
            ImGui::Text("Vendor '%s'.", cpuInfo.Vendor.c_str());
            ImGui::Text("Model '%s'."         , cpuInfo.Model.c_str());
            ImGui::Text("Architecture '%s'."  , Core::CPUArchitectureToString(cpuInfo.CPUArchitecture));
            ImGui::Text("Physical Cores '%u'.", cpuInfo.PhysicalCoreCount);
            ImGui::Text("Logical Cores '%u'." , cpuInfo.LogicalCoreCount);
            ImGui::Text("Speed GHz '%.2f'."   , speedInGHz);
            ImGui::Text("HyperThreaded '%s'." , cpuInfo.IsHyperThreaded ? "True" : "False");
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            Core::MemoryInformation memoryInfo = Platform::GetMemoryInformation();
            ImGui::Text("Memory Information");
            ImGui::Separator();

            ImGui::Indent(); 
            ImGui::Text("Process usage MB '%u'."        , Core::MemoryInformation::ToMB(memoryInfo.ProcessMemoryUsageBytes));
            ImGui::Text("Process Pages '%u'."           , memoryInfo.ProcessNumOfPagesCommitted);
            ImGui::Text("Process Page size KB '%u'."    , Core::MemoryInformation::ToKB(memoryInfo.PageSizeBytes));
            ImGui::Text("Total RAM MB '%u'."            , Core::MemoryInformation::ToMB(memoryInfo.TotalPhyscialMemoryBytes));
            ImGui::Text("Total Free RAM MB '%u'."       , Core::MemoryInformation::ToMB(memoryInfo.TotalPhyscialMemoryFreeBytes));
            ImGui::Text("Total Used RAM MB '%u'."       , Core::MemoryInformation::ToMB(memoryInfo.TotalPhyscialMemoryUsedBytes));
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Engine Information");
            ImGui::Separator();
            {
                ImGui::Indent();
                ImGui::Text("Engine Frame: %u", App::Engine::Instance().FrameCount);
                ImGui::Text("Render Frame: %u", Graphics::RenderContext::Instance().GetFrameCount());
                ImGui::Text("Engine Version: %s", App::Engine::EngineVersionToString().c_str());
                ImGui::Unindent();
            }
        }
    }
}