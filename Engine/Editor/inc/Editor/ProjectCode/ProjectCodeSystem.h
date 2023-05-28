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

        class ProjectCodeSystem : public Core::ISystem
        {
        public:
            ProjectCodeSystem();
            virtual ~ProjectCodeSystem() override;

            virtual void Initialise() override;
            virtual void Shutdown() override;

            IS_SYSTEM(ProjectCodeSystem)

        private:
            void ProjectOpened(Core::Event& e);
            void ProjectClosed(Core::Event& e);

            void GenerateProjectFiles();

        private:
            ProjectCodeInfo m_projectCodeInfo;
            Runtime::ProjectInfo m_projectInfo;
        };
    }
}