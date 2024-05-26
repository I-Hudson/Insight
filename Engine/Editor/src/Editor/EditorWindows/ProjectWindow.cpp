#include "Editor/EditorWindows/ProjectWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Runtime/ProjectSystem.h"
#include "Runtime/Engine.h"

#include "Core/Defines.h"
#include "Core/Profiler.h"

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
            : IEditorWindow(420, 640)
        { }

        ProjectWindow::ProjectWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(420, 640, maxWidth, maxHeight)
        { }

        ProjectWindow::~ProjectWindow()
        { }

        void ProjectWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            static u64 tabIndex = 0;
            constexpr const char* buttonLabels[] = { "Create Project", "Open Project" };

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
                    if (projectSystem->OpenProject(projectPath))
                    {
                        EditorWindowManager::Instance().RemoveWindow(WINDOW_NAME);
                    }
                }
            }
            else if (tabIndex == 1)
            {
                ImGui::InputText("Project Location", &projectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFile, &projectPath, { FileDialogFilter{ L"isproject", L"*.isproject"}});
                }

                if (ImGui::Button("Open"))
                {
                    if (projectSystem->OpenProject(projectPath))
                    {
                        EditorWindowManager::Instance().RemoveWindow(WINDOW_NAME);
                    }
                }
            }
        }
    }
}