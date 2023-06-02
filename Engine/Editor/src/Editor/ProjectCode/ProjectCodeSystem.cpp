#include "Editor/ProjectCode/ProjectCodeSystem.h"
#include "Editor/ProjectCode/PremakeProjectTemplate.h"
#include "Editor/ProjectCode/PremakeSolutionTemplate.h"

#include "Editor/EditorWindows/EditorWindowManager.h"

#include "Editor/HotReload/HotReloadMetaData.h"
#include "Editor/HotReload/HotReloadExportFunctions.h"

#include "Core/EnginePaths.h"
#include "Core/ImGuiSystem.h"

#include "Runtime/Engine.h"

#include "Event/EventSystem.h"
#include "FileSystem/FileSystem.h"

#include "Platforms/Platform.h"

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
        UnlinkProject();
        m_state = Core::SystemStates::Not_Initialised;
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
        GenerateProjectSolution(premakeSolutionFile);
        m_projectCodeInfo.PremakeSolutionFile = premakeSolutionFile;
    }

    void ProjectCodeSystem::BuildProject()
    {
        IS_CORE_WARN("[ProjectCodeSystem::BuildProject] To be implemented.");
    }

    void ProjectCodeSystem::LinkProject()
    {
        UnlinkProject();

#define PROJECT_LOAD_TESTING
#ifdef PROJECT_LOAD_TESTING
        std::string dynamicLibraryPath = m_projectInfo.GetIntermediatePath() + "/bin";
#if IS_PLATFORM_WINDOWS
#if IS_DEBUG
        dynamicLibraryPath += "/Debug-windows-";
#elif IS_RELEASE
        dynamicLibraryPath += "/Release-windows-";
#endif
        dynamicLibraryPath += "x86_64";
#endif
        dynamicLibraryPath += "/" + m_projectInfo.ProjectName;

        dynamicLibraryPath = FindNewestProjectDynamicLibrary(dynamicLibraryPath);

        m_projectDll = Platform::LoadDynamicLibrary(dynamicLibraryPath);
        if (m_projectDll)
        {
            FARPROC func = GetProcAddress((HMODULE)m_projectDll, "ProjectModuleInitialise");
            if (!func)
            {
                return;
            }

            auto initialiseFunc = Platform::GetDynamicFunction<void, Core::ImGuiSystem*>(m_projectDll, ProjectModule::c_Initialise);
            if (!initialiseFunc) { return; }

            Core::ImGuiSystem* imguiSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<Core::ImGuiSystem>();
            initialiseFunc(imguiSystem);

            auto getAllEditorWindowsNames = Platform::GetDynamicFunction<std::vector<std::string>>(m_projectDll, ProjectModule::c_GetEditorWindowNames);
            if (!getAllEditorWindowsNames) { return; }
            m_dllMetaData.RegisteredEditorWindows = getAllEditorWindowsNames();
            IS_CORE_INFO("[ProjectCodeSystem::LinkProject] Number of Project editor windows '{}'.", m_dllMetaData.RegisteredEditorWindows.size());

            auto getMetaData = Platform::GetDynamicFunction<HotReloadMetaData>(m_projectDll, ProjectModule::c_GetMetaData);
            if (!getMetaData) { return; }
            auto metaData = getMetaData();
            IS_CORE_INFO("[ProjectCodeSystem::LinkProject] Meta data loaded.");
        }
#endif
    }

    void ProjectCodeSystem::Reload()
    {
        ProjectReloadData reloadData;
        PreReload(reloadData);

        UnlinkProject();
        GenerateProjectFiles();
#if IS_PLATFORM_WINDOWS
        MSBuildProject(m_projectCodeInfo.SolutionFile);
#endif
        LinkProject();

        PostReload(reloadData);
    }

    void ProjectCodeSystem::ProjectOpened(Core::Event& e)
    {
        m_projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
        m_projectCodeInfo.ProjectFolder = m_projectInfo.GetIntermediatePath() + ProjectCodeInfo::c_CodeFolder;
        m_projectCodeInfo.ProjectFile = m_projectInfo.GetIntermediatePath() + ProjectCodeInfo::c_CodeFolder + "/" + m_projectInfo.ProjectName + ".vcxproj";
        m_projectCodeInfo.SolutionFile = m_projectInfo.GetProjectPath() + "/" + m_projectInfo.ProjectName + "_Solution.sln";
        GenerateProjectFiles();
    }

    void ProjectCodeSystem::ProjectClosed(Core::Event& e)
    {
        UnlinkProject();
    }

    void ProjectCodeSystem::GenerateProjectSolution(std::string_view solutionPath)
    {
        if (solutionPath.empty() || !FileSystem::Exists(solutionPath))
        {
            IS_CORE_ERROR("[ProjectCodeSystem::GenerateProjectSolution] Unable to generate solution as required premake file '{}' does not exist.", solutionPath);
            return;
        }

        std::string insightRootPath = EnginePaths::GetRootPath();

        std::string generateProjectsBatch = insightRootPath + "/Build/Engine/GENERATE_PROJECT.bat";
        std::string cmdCommend = generateProjectsBatch + " " + std::string(solutionPath) + " vs2019";
        std::system(cmdCommend.c_str());
    }

    void ProjectCodeSystem::PreReload(ProjectReloadData& reloadData) const
    {
        for (std::string_view view : m_dllMetaData.RegisteredEditorWindows)
        {
            bool windowIsActive = EditorWindowManager::Instance().GetActiveWindow(view) != nullptr;
            reloadData.EditorWindows.push_back(ProjectReloadData::EditorWindowReload(std::string(view), windowIsActive));
            EditorWindowManager::Instance().RemoveWindowNow(view);
        }
    }

    void ProjectCodeSystem::PostReload(const ProjectReloadData& reloadData) const
    {
        for (const ProjectReloadData::EditorWindowReload& window : reloadData.EditorWindows)
        {
            if (window.IsActive)
            {
                EditorWindowManager::Instance().AddWindow(window.Name);
            }
        }
    }

#if IS_PLATFORM_WINDOWS
    void ProjectCodeSystem::MSBuildProject(std::string_view solutionPath) const
    {
        std::string buildSolutionBatch = EnginePaths::GetRootPath() + "/Build/Engine/Build_Solution.bat";

#if IS_DEBUG
        std::string cmdCommend = buildSolutionBatch + " " + std::string(solutionPath) + " vs2022 Build Debug win64";
#elif IS_RELEASE
        std::string cmdCommend = buildSolutionBatch + " " + std::string(solutionPath) + " vs2022 Build Release win64";
#endif
        if (!cmdCommend.empty())
        {
            int buildErrorCode = std::system(cmdCommend.c_str());
            IS_CORE_INFO("[ProjectCodeSystem::MSBuildProject] Build error code: '{buildErrorCode}'.", );
        }
    }
#endif

    std::string ProjectCodeSystem::FindNewestProjectDynamicLibrary(std::string_view folderPath) const
    {   
        u64 newestWriteTime = _UI64_MAX;
        std::string filePath;

        for (auto path : std::filesystem::directory_iterator(folderPath))
        {
            u64 timeSinceWrite = path.last_write_time().time_since_epoch().count();
            if (FileSystem::GetFileExtension(path.path().generic_string()) == ".dll"
                && timeSinceWrite < newestWriteTime)
            {
                filePath = path.path().generic_string();
            }
            
        }
        return filePath;
    }

    void ProjectCodeSystem::UnlinkProject()
    {
        if (m_projectDll)
        {
            // Just do some clean up before unloading the dll.
            ProjectReloadData reloadData;
            PreReload(reloadData);

            auto uninitialiseFunc = Platform::GetDynamicFunction<void>(m_projectDll, "ProjectModuleUninitialise");
            if (uninitialiseFunc)
            {
                uninitialiseFunc();
            }

            Platform::FreeDynamicLibrary(m_projectDll);
            m_projectDll = nullptr;
        }
        m_dllMetaData = {};
    }
}