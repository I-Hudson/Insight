#include "Runtime/ProjectSystem.h"

#include "Core/Logger.h"

#include "FileSystem/FileSystem.h"
#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/JsonSerialiser.h"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <IconsFontAwesome5.h>

#include <fstream>
#include <filesystem>

namespace Insight
{
    namespace Runtime
    {
        constexpr char* c_InternalResourcePath = "/Resources";

        IS_SERIALISABLE_CPP(ProjectInfo)

        ProjectSystem::~ProjectSystem()
        {
        }

        void ProjectSystem::Initialise()
        {
#ifdef IS_PLATFORM_WINDOWS
            // Windows specific
            wchar_t szPath[MAX_PATH];
            GetModuleFileNameW(NULL, szPath, MAX_PATH);
            m_executablePath = std::move(std::filesystem::path{ szPath }.parent_path().string()); // to finish the folder path with (back)slash
            m_executablePath = FileSystem::FileSystem::PathToUnix(m_executablePath);
#endif

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

        void ProjectSystem::CreateProject(std::string_view projectPath)
        {
            if (FileSystem::FileSystem::Exists(projectPath))
            {
                IS_CORE_WARN("[ProjectSystem::CreateProject] Unable to create project at '{}'.", projectPath);
                return;
            }

            ProjectInfo project;
            project.ProjectPath = projectPath;
            project.ProjectName = projectPath;
            project.ProjectVersion = 0;

            GenerateProjectSolution();
        }

        void ProjectSystem::OpenProject(std::string projectPath)
        {
            /*std::string projectFilePath;
            PlatformFileDialog fileDialog;
            if (!fileDialog.ShowLoad(&projectFilePath, { { "Project (*.isproject)",  "*.isproject"} }))
            {
                return;
            }*/

            Archive projectArchive(projectPath, ArchiveModes::Read);
            std::vector<Byte> data = projectArchive.GetData();
            if (data.size() == 0)
            {
                return;
            }

            CloseProject();

            Serialisation::JsonSerialiser jsonSerialiser(true);
            jsonSerialiser.Deserialise(data);

            m_projectInfo.Deserialise(&jsonSerialiser);
            m_projectInfo.IsOpen = true;
        }

        void ProjectSystem::CloseProject()
        {
            m_projectInfo = { };
        }

        const ProjectInfo& ProjectSystem::GetProjectInfo() const
        {
            return m_projectInfo;
        }

        std::string ProjectSystem::GetExecutablePath() const
        {
            return m_executablePath;
        }

        std::string ProjectSystem::GetInternalResourcePath() const
        {
            return m_installLocation + c_InternalResourcePath;
        }

        bool ProjectSystem::CanCreateProject()
        {
            return true;
        }

        void ProjectSystem::GenerateProjectSolution()
        {
            std::string projectFullPath = m_projectInfo.ProjectPath + "/" + m_projectInfo.ProjectName + c_ProjectExtension;

            Serialisation::JsonSerialiser serialiser(false);
            u64 arraySize = 1;
            serialiser.StartArray("ProjectInfoStructs", arraySize);
            serialiser.SetVersion(1);
            m_projectInfo.Serialise(&serialiser);
            serialiser.StopArray();

            std::vector<Byte> serialisedData = serialiser.GetSerialisedData();
            serialiser = {};

            Archive archive(projectFullPath, ArchiveModes::Write);
            archive.Write(serialisedData.data(), serialisedData.size());
            archive.Close();

            archive = Archive(projectFullPath, ArchiveModes::Read);
            serialisedData = archive.GetData();
            Serialisation::JsonSerialiser deserialiserJson(true);
            deserialiserJson.Deserialise(serialisedData);
            ProjectInfo deserialiseInfo;

            u64 unused;
            deserialiserJson.StartArray("ProjectInfoStructs", unused);
            deserialiseInfo.Deserialise(&deserialiserJson);
            deserialiserJson.StopArray();
        }

        bool ProjectSystem::CanOpenProject()
        {
            return true;
        }
    }
}
