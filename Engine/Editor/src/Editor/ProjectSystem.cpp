#include "Editor/ProjectSystem.h"

#include "FileSystem/FileSystem.h"
#include "Platforms/Platform.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <IconsFontAwesome5.h>

#include <fstream>

namespace Insight
{
    namespace Editor
    {
        ProjectSystem::~ProjectSystem()
        {
        }

        void ProjectSystem::Initialise()
        {
            m_state = Core::SystemStates::Initialised;
        }

        void ProjectSystem::Shutdown()
        {
            CloseProject();
            m_state = Core::SystemStates::Not_Initialised;
        }

        bool ProjectSystem::IsProjectOpen() const
        {
            return m_projectInfo.IsOpen;
        }

        void ProjectSystem::OpenProject()
        {
        }

        void ProjectSystem::CloseProject()
        {
            m_projectInfo = { };
        }

        void ProjectSystem::Update()
        {
            if (IsProjectOpen())
            {
                return;
            }

            static int tabIndex = 0;
            constexpr const char* buttonLabels[] = { "Create Project", "Open Project" };

            ImGui::Begin("Project", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
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

            if (tabIndex == 0)
            {
                ImGui::InputText("Project Name", &m_projectInfo.ProjectName);
                ImGui::InputText("Project Location", &m_projectInfo.ProjectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFolder, &m_projectInfo.ProjectPath);
                }

                if (ImGui::Button("Create"))
                {
                    if (!CanCreateProject())
                    {
                        return;
                    }
                    // Generate a project file with all required information.
                    GenerateProjectSolution();
                    OpenProject();
                }
            }
            else if (tabIndex == 1)
            {
                ImGui::InputText("Project Location", &m_projectInfo.ProjectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFile, &m_projectInfo.ProjectPath, { PlatformDialogFileFilter(isproject, *.isproject)});
                }

                if (ImGui::Button("Open"))
                {
                    if (!CanOpenProject())
                    {
                        return;
                    }
                    OpenProject();
                }
            }

            ImGui::End();
        }

        bool ProjectSystem::CanCreateProject()
        {
            return true;
        }

        void ProjectSystem::GenerateProjectSolution()
        {
            
            Serialisation::SerialiserObject<ProjectInfo> serialise{};
            std::string serialisedData = serialise(m_projectInfo);

            std::string projectFullPath = m_projectInfo.ProjectPath + "/" + m_projectInfo.ProjectName + c_ProjectExtension;
            std::fstream stream{};
            stream.open(projectFullPath, std::ios::out | std::ios::trunc);
            if (stream.is_open())
            {
                stream.write(serialisedData.c_str(), serialisedData.size());
                stream.close();
            }
        }

        bool ProjectSystem::CanOpenProject()
        {
            return false;
        }
    }
}
