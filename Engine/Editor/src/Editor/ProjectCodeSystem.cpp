#include "Editor/ProjectCodeSystem.h"

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
        m_codeProjectFile = m_projectInfo.GetIntermediatePath() + c_CodeFolder + "/" + m_projectInfo.ProjectName + ".vcxproj";
        m_codeSolutionFile = m_projectInfo.GetProjectPath() + "/" + m_projectInfo.ProjectName + ".sln";

        if (!FileSystem::Exists(m_codeProjectFile) || !FileSystem::Exists(m_codeSolutionFile))
        {
            GenerateProjectFiles();
        }
    }

    void ProjectCodeSystem::ProjectClosed(Core::Event& e)
    {

    }
}