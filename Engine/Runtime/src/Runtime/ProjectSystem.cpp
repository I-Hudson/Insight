#include "Runtime/ProjectSystem.h"

#include "Asset/AssetRegistry.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "FileSystem/FileSystem.h"
#include "Platforms/Platform.h"

#include "Serialisation/Archive.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Event/EventSystem.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

namespace Insight
{
    namespace Runtime
    {
        IS_SERIALISABLE_CPP(ProjectInfo)

        ProjectSystem::~ProjectSystem()
        {
        }

        void ProjectSystem::Initialise()
        {
            ASSERT_MSG(AssetRegistry::IsValidInstance(), "[ProjectSystem::Initialise] There must be a valid AssetRegistry.");
            m_state = Core::SystemStates::Initialised;
        }

        void ProjectSystem::Shutdown()
        {
            IS_PROFILE_FUNCTION();

            Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<Core::ProjectCloseEvent>(m_projectInfo.ProjectPath));
            m_projectInfo = {};
            m_state = Core::SystemStates::Not_Initialised;
        }

        bool ProjectSystem::IsProjectOpen() const
        {
            return m_projectInfo.IsOpen;
        }

        bool ProjectSystem::CreateProject(std::string_view projectPath, std::string_view projectName)
        {
            ProjectInfo project;
            project.ProjectPath = projectPath;
            project.ProjectName = projectName;

            std::string projectFilePath = project.GetProjectFilePath();

            if (FileSystem::Exists(projectFilePath))
            {
                //IS_LOG_CORE_WARN("[ProjectSystem::CreateProject] Unable to create project at '{}'.", projectPath);
                //return false;
            }

            Serialisation::JsonSerialiser serialiser(false);
            serialiser.SetVersion(1);
            project.Serialise(&serialiser);

            std::vector<Byte> serialisedData = serialiser.GetSerialisedData();
            serialiser = {};

            Archive archive(projectFilePath, ArchiveModes::Write);
            archive.Write(serialisedData.data(), serialisedData.size());
            archive.Close();

            FileSystem::CreateFolder(project.GetContentPath());
            FileSystem::CreateFolder(project.GetIntermediatePath());

            Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<Core::ProjectCreateEvent>(m_projectInfo.ProjectPath));

            return true;
        }

        bool ProjectSystem::OpenProject(std::string projectPath)
        {
            if (!FileSystem::Exists(projectPath))
            {
                return false;
            }

            bool foundProjectFile = std::filesystem::path(projectPath).extension() == c_ProjectExtension;
            std::string isProjectPath;

            if (!foundProjectFile)
            {
                for (const auto& iter : std::filesystem::directory_iterator(projectPath))
                {
                    if (iter.path().extension() == c_ProjectExtension)
                    {
                        foundProjectFile = true;
                        projectPath = iter.path().string();
                        FileSystem::PathToUnix(projectPath);
                        break;
                    }
                }
            }
            else
            {
                isProjectPath = projectPath;
            }

            if (!foundProjectFile)
            {
                IS_LOG_CORE_WARN("[ProjectSystem::OpenProject] '{}' is not a valid project path. Please give the path to the '.isproject' file or folder that file is in.", projectPath);
                return false;
            }

            Archive projectArchive(projectPath, ArchiveModes::Read);
            std::vector<Byte> data = projectArchive.GetData();
            if (data.size() == 0)
            {
                return false;
            }

            //m_resourceSystem->ClearDatabase();

            if (m_projectInfo.IsOpen)
            {
                Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<Core::ProjectCloseEvent>(m_projectInfo.ProjectPath));
            }
            m_projectInfo = {};

            Serialisation::JsonSerialiser jsonSerialiser(true);
            if (!jsonSerialiser.Deserialise(data))
            {
                return false;
            }

            m_projectInfo.Deserialise(&jsonSerialiser);
            m_projectInfo.IsOpen = true;

            Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<Core::ProjectOpenEvent>(m_projectInfo.ProjectPath));
            return true;
        }

        void ProjectSystem::SaveProject()
        {
            Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<Core::ProjectSaveEvent>(m_projectInfo.ProjectPath));

            Serialisation::JsonSerialiser serialiser(false);
            serialiser.SetVersion(1);
            m_projectInfo.Serialise(&serialiser);

            std::vector<Byte> serialisedData = serialiser.GetSerialisedData();
            serialiser = {};

            Archive archive(m_projectInfo.GetProjectFilePath(), ArchiveModes::Write);
            archive.Write(serialisedData.data(), serialisedData.size());
            archive.Close();
        }

        const ProjectInfo& ProjectSystem::GetProjectInfo() const
        {
            ASSERT_MSG(IsProjectOpen(), "There must be an open project to get the project info.");
            return m_projectInfo;
        }
    }
}
