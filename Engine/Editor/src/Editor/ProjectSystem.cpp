#include "Editor/ProjectSystem.h"

#include "FileSystem/FileSystem.h"
#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/JsonSerialiser.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <IconsFontAwesome5.h>

#include <fstream>

namespace Insight
{
    namespace Editor
    {
        IS_SERIALISABLE_CPP(ProjectInfo)

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
            Archive archive(m_projectInfo.ProjectPath, ArchiveModes::Read);
            std::vector<Byte> data = archive.GetData();
            if (data.size() == 0)
            {
                return;
            }

            Serialisation::DeserialiserObject<ProjectInfo> deserialise{};
            ProjectInfo deserialisedInfo;
            //deserialise.Deserialise(std::string(data.begin(), data.end()), &deserialisedInfo);
            IS_UNUSED(deserialisedInfo);
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

            static u64 tabIndex = 0;
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
                    if (CanCreateProject())
                    {
                        // Generate a project file with all required information.
                        GenerateProjectSolution();
                        //OpenProject();
                    }
                }
            }
            else if (tabIndex == 1)
            {
                ImGui::InputText("Project Location", &m_projectInfo.ProjectPath);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER))
                {
                    PlatformFileDialog fileDialog;
                    fileDialog.Show(PlatformFileDialogOperations::SelectFile, &m_projectInfo.ProjectPath, { PlatformDialogFileFilter(isproject, *.isproject) });
                }

                if (ImGui::Button("Open"))
                {
                    if (CanOpenProject())
                    {
                        OpenProject();
                    }
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
            //Serialisation::SerialiserObject<ProjectInfo> serialise{};
            //for (int i = 0; i < 5; i++)
            //{
            //    m_projectInfo.IntTestArray.push_back(i);
            //    m_projectInfo.GUIDS.push_back(Core::GUID());
            //    m_projectInfo.ProjectPointerData.push_back(new ProjectPointerData{ 45 });
            //}
            //std::string serialisedData = serialise.Serialise(m_projectInfo);

            std::string projectFullPath = m_projectInfo.ProjectPath + "/" + m_projectInfo.ProjectName + c_ProjectExtension;

            Serialisation::JsonSerialiser serialiser(false);
            serialiser.SetVersion(1);
            m_projectInfo.Serialise(&serialiser);
            std::vector<Byte> serialisedData = serialiser.GetSerialisedData();

            Archive archive(projectFullPath, ArchiveModes::Write);
            archive.Write(serialisedData.data(), serialisedData.size());
            archive.Close();

            archive = Archive(projectFullPath, ArchiveModes::Read);
            serialisedData = archive.GetData();
            Serialisation::JsonSerialiser deserialiserJson(true);
            deserialiserJson.Deserialise(serialisedData);
            ProjectInfo deserialiseInfo;
            deserialiseInfo.Deserialise(deserialiserJson.GetChildSerialiser(0));
        }

        bool ProjectSystem::CanOpenProject()
        {
            return true;
        }
    }
}
