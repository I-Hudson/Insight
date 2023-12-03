#pragma once 

#include "Editor/HotReload/HotReloadLibrary.h"
#include "Editor/HotReload/HotReloadOperation.h"

#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include "Runtime/ProjectSystem.h"

#include "Event/Event.h"

#include <vector>
#include <string_view>

namespace Insight::Editor
{
    class HotReloadSystem : public Core::ISystem, public Core::Singleton<HotReloadSystem>
    {
    public:
        HotReloadSystem();
        virtual ~HotReloadSystem() override;

        IS_SYSTEM(HotReloadSystem)

        virtual void Initialise() override;
        virtual void Shutdown() override;

        const HotReloadLibrary& GetLibrary() const;

        void GenerateProjectSolution();
        /// @brief Initial a dll reload.
        void Reload();

    private:
        void BuildProjectSolution();

        void PreUnloadOperations();
        void PostLoadOperations();

        void LoadLibrary(std::string_view libraryPath);
        void UnloadLibrary();

        void OnProjectOpened(Core::Event& e);
        void OnProjectClosed(Core::Event& e);

        void RegisterAllHotReloadOperations();

        std::string GetLibraryPathFromProjectInfo(const Runtime::ProjectInfo& projectInfo) const;
        std::string FindNewestProjectDynamicLibrary(std::string_view folderPath) const;

    private:
        HotReloadLibrary m_library;
        std::vector<HotReloadOperation*> m_operations;
    };
}