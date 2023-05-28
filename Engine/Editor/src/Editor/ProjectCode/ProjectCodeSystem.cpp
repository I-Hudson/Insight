#include "Editor/ProjectCode/ProjectCodeSystem.h"
#include "Editor/ProjectCode/PremakeProjectTemplate.h"
#include "Editor/ProjectCode/PremakeSolutionTemplate.h"

#include "Core/EnginePaths.h"

#include "Event/EventSystem.h"
#include "FileSystem/FileSystem.h"

namespace Insight::Editor
{
    ProjectCodeSystem::ProjectCodeSystem()
    {
    }

    ProjectCodeSystem::~ProjectCodeSystem()
    {
    }

    void ProjectCodeSystem::Initialise()
    {
        Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Open, std::bind(&ProjectCodeSystem::ProjectOpened, this, std::placeholders::_1));
        Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Close, std::bind(&ProjectCodeSystem::ProjectClosed, this, std::placeholders::_1));
        m_state = Core::SystemStates::Initialised;
    }

    void ProjectCodeSystem::Shutdown()
    {
        Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Open);
        Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Close);
        m_state = Core::SystemStates::Not_Initialised;
    }

    void ProjectCodeSystem::ProjectOpened(Core::Event& e)
    {
        m_projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
        m_projectCodeInfo.ProjectFolder = m_projectInfo.GetIntermediatePath() + ProjectCodeInfo::c_CodeFolder;
        m_projectCodeInfo.ProjectFile = m_projectInfo.GetIntermediatePath() + ProjectCodeInfo::c_CodeFolder + "/" + m_projectInfo.ProjectName + ".vcxproj";
        m_projectCodeInfo.SolutionFile = m_projectInfo.GetProjectPath() + "/" + m_projectInfo.ProjectName + ".sln";

        if (!FileSystem::Exists(m_projectCodeInfo.ProjectFile) || !FileSystem::Exists(m_projectCodeInfo.SolutionFile))
        {
            GenerateProjectFiles();
        }
    }

    void ProjectCodeSystem::ProjectClosed(Core::Event& e)
    {

    }

    void ProjectCodeSystem::GenerateProjectFiles()
    {
        std::string insightRootPath = EnginePaths::GetRootPath();

        PremakeProjectTemplateData projectTemplateData;
        projectTemplateData.ProjectName = m_projectInfo.ProjectName.c_str();
        projectTemplateData.InsightRootPath = insightRootPath.c_str();

        CreatePremakeProjectTemplateFile(m_projectCodeInfo.ProjectFolder.c_str(), projectTemplateData);

        std::string projectPath = m_projectInfo.GetProjectPath();
        std::string solutionName = m_projectInfo.ProjectName + "_Solution";

        PremakeSolutionTemplateData solutionTemplateData;
        solutionTemplateData.SolutionName = solutionName.c_str();
        solutionTemplateData.ProjectName = m_projectInfo.ProjectName.c_str();
        solutionTemplateData.InsightRootPath = insightRootPath.c_str();

        std::string premakeSolutionFile = CreatePremakeSolutionTemplateFile(m_projectCodeInfo.ProjectFolder.c_str(), solutionTemplateData);

        std::string generateProjectsBatch = insightRootPath + "/Build/Engine/GENERATE_PROJECT.bat";
        std::string cmdCommend = generateProjectsBatch + " " + premakeSolutionFile + " vs2019";
        std::system(cmdCommend.c_str());
    }
}