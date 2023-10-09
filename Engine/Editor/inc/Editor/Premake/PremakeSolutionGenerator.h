#pragma once

#include "Editor/Premake/PremakeSolutionTemplateData.h"
#include "Editor/Premake/PremakeProjectTemplateData.h"

#include "Runtime/ProjectSystem.h"

namespace Insight::Editor
{
    struct PremakeHeaderToolData
    {
        std::vector<std::string> ReflectDirectories;
        std::string GeneratedFilesOutputPath;
    };

    struct PremakeCreateFunctions
    {
        CreatePremakeSolutionTemplateFileFunc CreateSolutionFunc = nullptr;
        CreatePremakeProjectTemplateFileFunc CreateProjectFunc = nullptr;

        operator bool() const
        {
            return CreateSolutionFunc != nullptr
                && CreateProjectFunc != nullptr;
        }
    };

    struct PremakeTemplateData
    {
        PremakeHeaderToolData HeaderToolData;
        PremakeSolutionTemplateData SolutionData;
        PremakeProjectTemplateData ProjectData;
        PremakeCreateFunctions CreateFuncs;
    };


    class PremakeSolutionGenerator
    {
    public:
        bool GenerateSolution(const PremakeTemplateData& templateData) const;
        bool BuildSolution(const char* solutionPath, const char* outputDirectory = "") const;

        static std::string GetProjectIntermediateCodePath();
        static std::string GetProjectIDESolutionName();
        static std::string GetProjectIDESolutionPath();

    private:
        void RunInsightHeaderTool(const PremakeHeaderToolData& headerToolData) const;
        void GenerateProjectPremake(const PremakeProjectTemplateData& projectData, const CreatePremakeProjectTemplateFileFunc& createProjectFunc) const;
        std::string GenerateSolutionPremake(const PremakeSolutionTemplateData& solutionData, const CreatePremakeSolutionTemplateFileFunc& createSolutionFunc) const;

        void GenerateIDESolution(std::string_view solutionPath) const;
    };
}