#include "Editor/EditorWindows/ProjectSettingsWindow.h"
#include"Editor/EditorGUI.h"

#include "Runtime/ProjectSystem.h"
#include "World/WorldSystem.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Insight
{
    namespace Editor
    {
        ProjectSettingsWindow::ProjectSettingsWindow()
            : IEditorWindow()
        { }

        ProjectSettingsWindow::ProjectSettingsWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        ProjectSettingsWindow::ProjectSettingsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        ProjectSettingsWindow::~ProjectSettingsWindow()
        { }

        void ProjectSettingsWindow::OnDraw()
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            if (!projectInfo.IsOpen)
            {
                return;
            }

            ImGui::Text("Name");
            ImGui::SameLine();
            ImGui::Text("%s", projectInfo.ProjectName.c_str());

            ImGui::Text("Path");
            ImGui::SameLine();
            ImGui::Text("%s", projectInfo.ProjectPath.c_str());

            ImGui::Text("Version");
            ImGui::SameLine();
            ImGui::Text("%u", projectInfo.ProjectVersion);

            Reflect::TypeInfo worldTypeInfo = Runtime::World::GetStaticTypeInfo();
            Runtime::World* defaultWorld = Runtime::WorldSystem::Instance().GetWorldFromGuid(projectInfo.DefaultWorld).Get();

            std::string_view worldName = defaultWorld != nullptr ? defaultWorld->GetWorldName() : "";
            ImGui::Text("Default World");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1));
            ImGui::InputText("##DefaultWorld", (char*)worldName.data(), worldName.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopStyleColor();
            //EditorGUI::ObjectFieldTarget("DragDropTarget", "Default World", defaultWorld, worldTypeInfo.GetType());
        }
    }
}