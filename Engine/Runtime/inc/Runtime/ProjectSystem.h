#pragma once

#include "Core/GUID.h"
#include "Core/ISysytem.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

namespace Insight
{
    namespace Runtime
    {
        constexpr const char* c_ProjectExtension = ".isproject";

        struct ProjectInfo : public Serialisation::ISerialisable
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;
           
            const std::string& GetProjectPath() const { return ProjectPath; }
            std::string GetContentPath() const { return ProjectPath + "/Content"; }
            std::string GetIntermediatePath() const { return ProjectPath + "/Intermediate"; }

            IS_SERIALISABLE_H(ProjectInfo);
        };

        class ProjectSystem : public Core::ISystem
        {
        public:
            virtual ~ProjectSystem() override;

            virtual void Initialise()override;
            virtual void Shutdown() override;

            IS_SYSTEM(ProjectSystem);

            bool IsProjectOpen() const;

            void CreateProject(std::string_view projectPath);
            void OpenProject(std::string projectPath);
            void CloseProject();

            const ProjectInfo& GetProjectInfo() const;

            std::string GetExecutablePath() const;
            std::string GetInternalResourcePath() const;

        private:
            /// @brief Verify that a project can be created in the path given.
            /// @param projectPath 
            /// @return bool
            bool CanCreateProject();
            void GenerateProjectSolution();
            bool CanOpenProject();

        private:
            ProjectInfo m_projectInfo;

            std::string m_executablePath;
            std::string m_installLocation = "./";
        };
    }

    OBJECT_SERIALISER(Runtime::ProjectInfo, 2,
        SERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
        SERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
        SERIALISE_PROPERTY(u32, ProjectVersion, 1, 2)
        SERIALISE_PROPERTY(bool, IsOpen, 1, 2)
    );
}