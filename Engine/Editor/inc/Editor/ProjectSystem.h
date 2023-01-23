#pragma once

#include "Core/GUID.h"
#include "Core/ISysytem.h"

#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_ProjectExtension = ".isproject";

        struct BaseBaseProjectInfo
        {
            std::vector<Core::GUID> GUIDS;
        };

        struct BaseProjectInfo : public BaseBaseProjectInfo
        {
            Core::GUID GUID;
        };

        struct ProjectPointerData
        {
            int Age; // Project age
        };

        struct ProjectInfo : public BaseProjectInfo
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;

            std::vector<int> IntTestArray;
            std::vector<ProjectPointerData*> ProjectPointerData;

            std::string GetAbsPathWithFile() const { return ProjectPath + "/" + ProjectName; }
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

    OBJECT_SERIALISER(Editor::BaseBaseProjectInfo, 1,
        SERIALISE_PROPERTY_VECTOR(Core::GUID, GUIDS, 1, 0)
    );
    OBJECT_DESERIALISER(Editor::BaseBaseProjectInfo, 1,
        DESERIALISE_PROPERTY_VECTOR(Core::GUID, GUIDS, 1, 0)
    );

    OBJECT_SERIALISER(Editor::BaseProjectInfo, 2,
        SERIALISE_PARENT(Editor::BaseBaseProjectInfo, BaseBaseProjectInfo, 2, 0)
        SERIALISE_OBJECT(Core::GUID, GUID, 1, 0)
    );
    OBJECT_DESERIALISER(Editor::BaseProjectInfo, 2,
        DESERIALISE_PARENT(Editor::BaseBaseProjectInfo, BaseBaseProjectInfo, 2, 0)
        DESERIALISE_OBJECT(Core::GUID, GUID, 1, 0)
    );

    OBJECT_SERIALISER(Editor::ProjectPointerData, 1,
        SERIALISE_OBJECT(int, Age, 1, 0)
    );
    OBJECT_DESERIALISER(Editor::ProjectPointerData, 1,
        DESERIALISE_OBJECT(int, Age, 1, 0)
    );

    OBJECT_SERIALISER(Editor::ProjectInfo, 3,
        SERIALISE_OBJECT_VECTOR(Editor::ProjectPointerData*, ProjectPointerData, 3, 0)
        SERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
        SERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        SERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        SERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        SERIALISE_OBJECT(bool, IsOpen,         1, 0)
        SERIALISE_PROPERTY_VECTOR(int, IntTestArray,   1, 0)
        );
    OBJECT_DESERIALISER(Editor::ProjectInfo, 3,
        DESERIALISE_OBJECT_VECTOR(Editor::ProjectPointerData*, ProjectPointerData, 3, 0)
        DESERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
        DESERIALISE_OBJECT(std::string, ProjectPath, 1, 0)
        DESERIALISE_OBJECT(std::string, ProjectName, 1, 0)
        DESERIALISE_OBJECT(u32, ProjectVersion, 1, 0)
        DESERIALISE_OBJECT(bool, IsOpen, 1, 0)
        DESERIALISE_PROPERTY_VECTOR(int, IntTestArray, 1, 0)
    );


    //SERIALISER_BEGIN(Editor::ProjectInfo, 1);
    //    SERIALISER_OBJECT(std::string,                ProjectPath,    1, 0);
    //    SERIALISER_OBJECT(std::string,                ProjectName,    1, 0);
    //    SERIALISER_OBJECT(u32,                        ProjectVersion, 1, 0);
    //    SERIALISER_OBJECT(bool,                       IsOpen,         1, 0);
    //    SERIALISER_VECTOR(int,                        IntTestArray,   1, 0);
    //SERIALISER_END();
}