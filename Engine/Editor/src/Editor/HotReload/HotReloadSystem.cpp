#include "Editor/HotReload/HotReloadSystem.h"
#include "Editor/HotReload/HotReloadSolutionGenerator.h"

#include "Core/Logger.h"

#include "Event/EventSystem.h"
#include "Algorithm/Vector.h"
#include "FileSystem/FileSystem.h"

#include <filesystem>

#undef LoadLibrary

namespace Insight::Editor
{

    HotReloadSystem::HotReloadSystem()
    {
    }

    HotReloadSystem::~HotReloadSystem()
    {
    }

    void HotReloadSystem::Initialise()
    {
        Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Open, std::bind(&HotReloadSystem::OnProjectOpened, this, std::placeholders::_1));
        Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Close, std::bind(&HotReloadSystem::OnProjectClosed, this, std::placeholders::_1));
        m_state = Core::SystemStates::Initialised;
    }

    void HotReloadSystem::Shutdown()
    {
        Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Open);
        Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Close);
        UnloadLibrary();
        m_state = Core::SystemStates::Not_Initialised;
    }

    void HotReloadSystem::GenerateProjectSolution()
    {
        const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
        HotReloadSolutionGenerator solutionGenerator;
        solutionGenerator.GenerateSolution(projectInfo);
    }

    void HotReloadSystem::Reload()
    {
        UnloadLibrary();

        GenerateProjectSolution();

        LoadLibrary(GetLibraryPathFromProjectInfo(Runtime::ProjectSystem::Instance().GetProjectInfo()));
    }

    void HotReloadSystem::BuildProjectSolution()
    {
        GenerateProjectSolution();
        const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
        HotReloadSolutionGenerator solutionGenerator;
        solutionGenerator.BuildSolution(projectInfo);
    }

    void HotReloadSystem::PreUnloadOperations() const
    {
    }

    void HotReloadSystem::PostLoadOperations() const
    {
    }

    void HotReloadSystem::LoadLibrary(std::string_view libraryPath)
    {
        m_library.Load(libraryPath);
        PostLoadOperations();
    }

    void HotReloadSystem::UnloadLibrary()
    {
        PreUnloadOperations();
        m_library.Unload();
    }

    void HotReloadSystem::OnProjectOpened(Core::Event& e)
    {
        const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

        // Generate project and solution,
        HotReloadSolutionGenerator solutionGenerator;
        solutionGenerator.GenerateSolution(projectInfo);
        // Build project and solution,
        solutionGenerator.BuildSolution(projectInfo);

        LoadLibrary(GetLibraryPathFromProjectInfo(projectInfo));
    }

    void HotReloadSystem::OnProjectClosed(Core::Event& e)
    {
        UnloadLibrary();
    }

    std::string HotReloadSystem::GetLibraryPathFromProjectInfo(const Runtime::ProjectInfo& projectInfo) const
    {
        std::string dynamicLibraryPath = projectInfo.GetIntermediatePath() + "/bin";
#if IS_PLATFORM_WINDOWS
#if IS_DEBUG
        dynamicLibraryPath += "/Debug-windows-";
#elif IS_RELEASE
        dynamicLibraryPath += "/Release-windows-";
#endif
        dynamicLibraryPath += "x86_64";
#endif
        dynamicLibraryPath += "/" + projectInfo.ProjectName;
        return FindNewestProjectDynamicLibrary(dynamicLibraryPath);
    }

    std::string HotReloadSystem::FindNewestProjectDynamicLibrary(std::string_view folderPath) const
    {
        u64 newestWriteTime = _UI64_MAX;
        std::string filePath;

#if IS_PLATFORM_WINDOWS
#define DLL_EXTENSION ".dll"
#elif IS_PLATFORM_LINUX
#define DLL_EXTENSION ".dll"
#elif IS_PLATFORM_XBOX
#define DLL_EXTENSION ".dll"
#endif
        if (!FileSystem::Exists(folderPath))
        {
            IS_CORE_ERROR("[HotReloadSystem::FindNewestProjectDynamicLibrary] Folder path '{}' doesn't exists. Make sure the project solution is building correctly.", folderPath);
            return "";
        }

        for (auto path : std::filesystem::directory_iterator(folderPath))
        {
            u64 timeSinceWrite = path.last_write_time().time_since_epoch().count();
            if (FileSystem::GetFileExtension(path.path().generic_string()) == DLL_EXTENSION
                && timeSinceWrite < newestWriteTime)
            {
                filePath = path.path().generic_string();
            }

        }
        return filePath;
    }
}
