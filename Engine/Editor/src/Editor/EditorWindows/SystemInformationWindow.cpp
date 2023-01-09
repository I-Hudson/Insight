#include "Editor/EditorWindows/SystemInformationWindow.h"

#include "Platforms/Platform.h"

#include <imgui.h>

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
            Core::CPUInformation cpuInfo = Platform::GetCPUInformation();
            float speedInGHz = static_cast<float>(cpuInfo.SpeedInMHz) / 1000;

            ImGui::Text("CPU Vendor '%s'."       , cpuInfo.Vendor.c_str());
            ImGui::Text("CPU Model '%s'."        , cpuInfo.Model.c_str());
            ImGui::Text("CPU Architecture '%s'." , Core::CPUArchitectureToString(cpuInfo.CPUArchitecture));
            ImGui::Text("CPU Speed GHz '%f'."    , speedInGHz);
            ImGui::Text("CPU HyperThreaded '%s'.", cpuInfo.IsHyperThreaded ? "True" : "False");
        }
    }
}