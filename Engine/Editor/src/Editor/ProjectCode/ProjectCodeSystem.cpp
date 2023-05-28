#include "Editor/ProjectCode/ProjectCodeSystem.h"
#include "Editor/ProjectCode/PremakeProjectTemplate.h"
#include "Editor/ProjectCode/PremakeSolutionTemplate.h"

#include "Core/EnginePaths.h"
#include "Core/ImGuiSystem.h"

#include "Runtime/Engine.h"

#include "Event/EventSystem.h"
#include "FileSystem/FileSystem.h"

#include <Windows.h>

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

        std::string reflectProjectCommand = "" + insightRootPath + "/bin/Release-windows-x86_64/InsightReflectTool/InsightReflectTool.exe";
        reflectProjectCommand += " Type=Project";
        reflectProjectCommand += " ParsePath=" + m_projectInfo.GetContentPath();
        reflectProjectCommand += " GenerateProjectFileOutputPath=" + m_projectCodeInfo.ProjectFolder + "/Generated";
        std::system(reflectProjectCommand.c_str());

        PremakeProjectTemplateData projectTemplateData;
        projectTemplateData.ProjectName = m_projectInfo.ProjectName.c_str();
        projectTemplateData.InsightRootPath = insightRootPath.c_str();
        projectTemplateData.AdditionalFiles.push_back(m_projectCodeInfo.ProjectFolder + "/Generated");

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

//#define PROJECT_LOAD_TESTING
#ifdef PROJECT_LOAD_TESTING
        HMODULE projectLibrary = LoadLibrary(L"E:\\User\\Documents\\SourceControl\\Github\\C++Porjects\\Insight\\DemoProject\\Intermediate\\bin\\Debug-windows-x86_64\\DemoProject\\DemoProject.dll");
        if (projectLibrary)
        {
            FARPROC func = GetProcAddress(projectLibrary, "ProjectModuleInitialise");
            if (!func)
            {
                return;
            }

            using ProjectInitaliseFunc = void(*)(Core::ImGuiSystem*);
            ProjectInitaliseFunc initialiseFunc = reinterpret_cast<ProjectInitaliseFunc>(func);
            if (!initialiseFunc)
            {
                return;
            }

            Core::ImGuiSystem* imguiSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<Core::ImGuiSystem>();
            initialiseFunc(imguiSystem);
        }
#endif
    }
}