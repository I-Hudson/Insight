#pragma once

#include "Core/GUID.h"
#include "Core/ISysytem.h"

#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_ProjectExtension = ".isproject";

        struct BaseProjectInfo
        {
            Core::GUID GUID;
        };

        struct ProjectInfo : public BaseProjectInfo
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;

            std::vector<int> IntTestArray;
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

    OBJECT_SERIALISER(Editor::BaseProjectInfo, 1,
        SERIALISE_OBJECT(Core::GUID, GUID, 1, 0)
    );
    OBJECT_DESERIALISER(Editor::BaseProjectInfo, 1,
        DESERIALISE_OBJECT(Core::GUID, GUID, 1, 0)
    );

    OBJECT_SERIALISER(Editor::ProjectInfo, 2,
        SERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
        SERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        SERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        SERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        SERIALISE_OBJECT(bool, IsOpen,         1, 0)
        SERIALISE_VECTOR(int, IntTestArray,   1, 0)
        );
    OBJECT_DESERIALISER(Editor::ProjectInfo, 2,
        DESERIALISE_PARENT(Editor::BaseProjectInfo, 2, 0)
        DESERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        DESERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        DESERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        DESERIALISE_OBJECT(bool, IsOpen, 1, 0)
        DESERIALISE_VECTOR(int, IntTestArray, 1, 0)
    );


    //SERIALISER_BEGIN(Editor::ProjectInfo, 1);
    //    SERIALISER_OBJECT(std::string,                ProjectPath,    1, 0);
    //    SERIALISER_OBJECT(std::string,                ProjectName,    1, 0);
    //    SERIALISER_OBJECT(u32,                        ProjectVersion, 1, 0);
    //    SERIALISER_OBJECT(bool,                       IsOpen,         1, 0);
    //    SERIALISER_VECTOR(int,                        IntTestArray,   1, 0);
    //SERIALISER_END();
}