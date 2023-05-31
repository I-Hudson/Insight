#pragma once

#include "Core/ISysytem.h"
#include "Event/Event.h"

#include "Runtime/ProjectSystem.h"

#include <string>

namespace Insight
{
    namespace Editor
    {
        struct ProjectCodeInfo
        {
            constexpr static const char* c_CodeFolder = "/CodeFiles";

            std::string ProjectFile;
            std::string SolutionFile;

            std::string ProjectFolder;
        };

        /// @brief Store general meta data about the loaded DLL which can be used by the engine.
        struct ProjectDLLMetaData
        {
            std::vector<std::string> RegisteredEditorWindows;
            std::vector<std::string> RegisteredComponents;
        };

        class ProjectCodeSystem : public Core::ISystem, public Core::Singleton<ProjectCodeSystem>
        {
        public:
            ProjectCodeSystem();
            virtual ~ProjectCodeSystem() override;

            virtual void Initialise() override;
            virtual void Shutdown() override;

            IS_SYSTEM(ProjectCodeSystem)

            void GenerateProjectFiles();
            void BuildProject();
            void LinkProject();
            void UnlinkProject();

        private:
            void ProjectOpened(Core::Event& e);
            void ProjectClosed(Core::Event& e);

            void GenerateProjectSolution(std::string_view solutionPath);

            std::string FindNewestProjectDynamicLibrary(std::string_view folderPath);

        private:
            ProjectCodeInfo m_projectCodeInfo;
            Runtime::ProjectInfo m_projectInfo;
            ProjectDLLMetaData m_dllMetaData;

            void* m_projectDll = nullptr;
        };
    }
}