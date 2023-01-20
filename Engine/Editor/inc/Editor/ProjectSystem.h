#pragma once

#include "Core/ISysytem.h"

#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_ProjectExtension = ".isproject";

        struct ProjectInfo
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;
        };

        class ProjectSystem : public Core::ISystem
        {
        public:
            virtual ~ProjectSystem() override;

            virtual void Initialise()override;
            virtual void Shutdown() override;

            IS_SYSTEM(ProjectSystem);

            bool IsProjectOpen() const;
            void OpenProject();
            void CloseProject();

            void Update();

        private:
            /// @brief Verify that a project can be created in the path given.
            /// @param projectPath 
            /// @return bool
            bool CanCreateProject();
            void GenerateProjectSolution();
            bool CanOpenProject();

        private:
            ProjectInfo m_projectInfo;
        };
    }

    SERIALISER_BEGIN(Editor::ProjectInfo, 1);
        SERIALISER_PROPERTY(std::string,    ProjectPath,    1, 0);
        SERIALISER_PROPERTY(std::string,    ProjectName,    1, 0);
        SERIALISER_PROPERTY(u32,            ProjectVersion, 1, 0);
        SERIALISER_PROPERTY(bool,           IsOpen,         1, 0);
    SERIALISER_END();
}