#pragma once

#include "Runtime/Defines.h"

#include "Core/GUID.h"
#include "Core/ISysytem.h"
#include "Core/Singleton.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

namespace Insight
{
    namespace Runtime
    {
        class ResourceSystem;

        constexpr const char* c_ProjectExtension = ".isproject";

        //TODO High: Move this into the editor project and out of the Runtime one.
        struct IS_RUNTIME ProjectInfo : public Serialisation::ISerialisable
        {
            std::string ProjectPath;
            std::string ProjectName;
            u32 ProjectVersion = 0;
            bool IsOpen = false;
            Core::GUID DefaultWorld;

            const std::string& GetProjectPath() const { return ProjectPath; }
            std::string GetProjectFilePath() const { return ProjectPath + "/" + ProjectName + c_ProjectExtension; }
            std::string GetContentPath() const { return ProjectPath + "/Content"; }
            std::string GetIntermediatePath() const { return ProjectPath + "/Intermediate"; }

            IS_SERIALISABLE_H(ProjectInfo);
        };

        class IS_RUNTIME ProjectSystem : public Core::ISystem, public Core::Singleton<ProjectSystem>
        {
        public:
            virtual ~ProjectSystem() override;

            virtual void Initialise() override;
            virtual void Shutdown() override;

            IS_SYSTEM(ProjectSystem);

            void SetResourceSystem(ResourceSystem* resourceSystem);

            bool IsProjectOpen() const;

            bool CreateProject(std::string_view projectPath, std::string_view projectName);
            bool OpenProject(std::string projectPath);

            void SaveProject();

            const ProjectInfo& GetProjectInfo() const;

        private:
            ProjectInfo m_projectInfo;
            ResourceSystem* m_resourceSystem = nullptr;
        };
    }

    OBJECT_SERIALISER(Runtime::ProjectInfo, 3,
        SERIALISE_PROPERTY(std::string, ProjectPath, 1, 0)
        SERIALISE_PROPERTY(std::string, ProjectName, 1, 0)
        SERIALISE_PROPERTY(u32, ProjectVersion, 1, 2)
        SERIALISE_PROPERTY(bool, IsOpen, 1, 2)
        SERIALISE_PROPERTY(Core::GUID, DefaultWorld, 3, 0)
    );
}