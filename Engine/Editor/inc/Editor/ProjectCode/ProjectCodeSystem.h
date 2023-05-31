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

            std::string PremakeSolutionFile;

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

        /// @brief Store data in relation to when the project dll is being reloaded.
        struct ProjectReloadData
        {
            struct EditorWindowReload
            {
                EditorWindowReload() { }
                EditorWindowReload(std::string name, bool isActive)
                    : Name(std::move(name)), IsActive(isActive)
                { }

                std::string Name;
                bool IsActive = false;
            };
            struct ComponentReload
            {
                std::string Name;
            };

            std::vector<EditorWindowReload> EditorWindows;
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

            /// @brief Attempted to unlink the old project dll and link a new project dll
            /// while preserving data between them.
            void Reload();

        private:
            void ProjectOpened(Core::Event& e);
            void ProjectClosed(Core::Event& e);

            void GenerateProjectSolution(std::string_view solutionPath);

            /// @brief Perform some clean up before the project dll is removed.
            /// @param reloadData 
            void PreReload(ProjectReloadData& reloadData) const;
            /// @brief Perform recreation of objects (EditorWindows/Components) after a new 
            /// project dll is loaded (try and persist data across project dll reloads).
            /// @param reloadData 
            void PostReload(const ProjectReloadData& reloadData) const;

#if IS_PLATFORM_WINDOWS
            void MSBuildProject(std::string_view solutionPath) const;
#endif
            std::string FindNewestProjectDynamicLibrary(std::string_view folderPath) const;

        private:
            ProjectCodeInfo m_projectCodeInfo;
            Runtime::ProjectInfo m_projectInfo;
            ProjectDLLMetaData m_dllMetaData;

            void* m_projectDll = nullptr;
        };
    }
}