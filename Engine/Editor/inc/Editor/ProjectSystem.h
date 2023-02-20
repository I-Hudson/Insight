#pragma once

#include "Core/GUID.h"
#include "Core/ISysytem.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_ProjectExtension = ".isproject";

        struct BaseBaseProjectInfo
        {
            std::vector<Core::GUID> GUIDS;
        };

        struct BaseProjectInfo : public Serialisation::ISerialisable, public BaseBaseProjectInfo
        {
            Core::GUID GUID;

            IS_SERIALISABLE_H(BaseProjectInfo)
        };

        struct ProjectPointerData : public Serialisation::ISerialisable
        {
            int Age; // Project age
            
            IS_SERIALISABLE_H(ProjectPointerData)
        };

        struct ProjectInfo : public BaseProjectInfo
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;
            ProjectPointerData Data;

            std::vector<BaseProjectInfo> BaseProjectInfoTestArray;
            std::vector<std::shared_ptr<BaseProjectInfo>> BaseProjectInfoTestSharedPtrArray;
            std::vector<ProjectPointerData*> ProjectPointerData;

            std::string GetAbsPathWithFile() const { return ProjectPath + "/" + ProjectName; }

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

    namespace Serialisation
    {
        struct BaseProjectInfoToHash {};
        template<>
        struct PropertySerialiser<BaseProjectInfoToHash>
        {
            using InType = Editor::BaseProjectInfo;
            u64 operator()(InType& v) const
            {
                //return Algorithm::GetHash64(v.GUID.m_bytes, Core::GUID::c_GUID_BYTE_SIZE);
                return 0;
            }
        };

        template<>
        struct PropertyDeserialiser<BaseProjectInfoToHash>
        {
            using InType = u64;
            std::shared_ptr<Editor::BaseProjectInfo> operator()(u64) const
            {
                return std::make_shared<Editor::BaseProjectInfo>();
            }
        };
    }

    //OBJECT_SERIALISER(Editor::BaseBaseProjectInfo, 1,
    //    SERIALISE_PROPERTY_VECTOR(Core::GUID, GUIDS, 1, 0)
    //);
    //OBJECT_DESERIALISER(Editor::BaseBaseProjectInfo, 1,
    //    DESERIALISE_PROPERTY_VECTOR(Core::GUID, GUIDS, 1, 0)
    //);

    OBJECT_SERIALISER(Editor::BaseProjectInfo, 1,
        SERIALISE_PROPERTY(Core::GUID, GUID, 1, 0)
    );
    //OBJECT_DESERIALISER(Editor::BaseProjectInfo, 2,
    //    DESERIALISE_PARENT(Editor::BaseBaseProjectInfo, BaseBaseProjectInfo, 2, 0)
    //    DESERIALISE_PROPERTY(Core::GUID, GUID, 1, 0)
    //);

    OBJECT_SERIALISER(Editor::ProjectPointerData, 1,
        SERIALISE_PROPERTY(int, Age, 1, 0)
    );
    //OBJECT_DESERIALISER(Editor::ProjectPointerData, 1,
    //    DESERIALISE_PROPERTY(int, Age, 1, 0)
    //);

    OBJECT_SERIALISER(Editor::ProjectInfo, 2,
        SERIALISE_BASE(Editor::BaseProjectInfo, 1, 0)
        SERIALISE_OBJECT(Editor::ProjectPointerData, Data, 2, 0)
        SERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
        SERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
        SERIALISE_PROPERTY(u32, ProjectVersion, 1, 0)
        SERIALISE_PROPERTY(bool, IsOpen, 1, 0)
        SERIALISE_VECTOR_OBJECT(Editor::BaseProjectInfo, BaseProjectInfoTestArray, 1, 0)
        SERIALISE_VECTOR_PROPERTY(BaseProjectInfoToHash, BaseProjectInfoTestSharedPtrArray, 1, 0)

        //SERIALISE_OBJECT_VECTOR(Editor::ProjectPointerData*, ProjectPointerData, 3, 0)
        //SERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
        //SERIALISE_PROPERTY_VECTOR(int, IntTestArray,   1, 0)
    );

    //OBJECT_DESERIALISER(Editor::ProjectInfo, 3,
    //    DESERIALISE_OBJECT_VECTOR(Editor::ProjectPointerData*, ProjectPointerData, 3, 0)
    //    DESERIALISE_PARENT(Editor::BaseProjectInfo, BaseProjectInfo, 2, 0)
    //    DESERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
    //    DESERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
    //    DESERIALISE_PROPERTY(u32, ProjectVersion, 1, 0)
    //    DESERIALISE_PROPERTY(bool, IsOpen, 1, 0)
    //    DESERIALISE_PROPERTY_VECTOR(int, IntTestArray, 1, 0)
    //);
}