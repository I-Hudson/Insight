#include "Editor/EditorWindows/ProjectWindow.h"

#include "Runtime/ProjectSystem.h"
#include "Core/Defines.h"

#include "Runtime/Engine.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <IconsFontAwesome5.h>

namespace Insight
{
    namespace Editor
    {
        ProjectWindow::ProjectWindow()
            : IEditorWindow()
        { }

        ProjectWindow::ProjectWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        ProjectWindow::ProjectWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        ProjectWindow::~ProjectWindow()
        { }

        void ProjectWindow::OnDraw()
        {
            static u64 tabIndex = 0;
            constexpr const char* buttonLabels[] = { "Create Project", "Open Project" };

            //ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
            for (size_t buttonIdx = 0; buttonIdx < ARRAY_COUNT(buttonLabels); ++buttonIdx)
            {
                if (ImGui::Button(buttonLabels[buttonIdx]))
                {
                    tabIndex = buttonIdx;
                    break;
                }
                if (buttonIdx < ARRAY_COUNT(buttonLabels) - 1)
                {
                    ImGui::SameLine();
                }
            }

            static std::string projectPath;
            static std::string projectName;

            Runtime::ProjectSystem* projectSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::ProjectSystem>();
            ASSERT(projectSystem);

            if (tabIndex == 0)
            {
                ImGui::InputText("Project Name", &projectName);
                ImGui::InputText("Project Location", &projectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFolder, &projectPath);
                }

                if (ImGui::Button("Create"))
                {
                    projectSystem->CreateProject(projectPath, projectName);
                    projectSystem->OpenProject(projectPath);
                }
            }
            else if (tabIndex == 1)
            {
                ImGui::InputText("Project Location", &projectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFile, &projectPath, { PlatformDialogFileFilter(isproject, *.isproject) });
                }

                if (ImGui::Button("Open"))
                {
                    projectSystem->OpenProject(projectPath);
                }
            }

            //ImGui::End();
        }
    }
}