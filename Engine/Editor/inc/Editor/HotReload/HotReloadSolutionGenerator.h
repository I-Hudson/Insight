#pragma once

#include "Runtime/ProjectSystem.h"

namespace Insight::Editor
{
    class HotReloadSolutionGenerator
    {
    public:
        bool GenerateSolution(const Runtime::ProjectInfo& projectInfo) const;
        bool BuildSolution(const Runtime::ProjectInfo& projectInfo) const;

    private:
        void RunInsightHeaderTool(const Runtime::ProjectInfo& projectInfo) const;
        void GenerateProjectPremake(const Runtime::ProjectInfo& projectInfo) const;
        std::string GenerateSolutionPremake(const Runtime::ProjectInfo& projectInfo) const;

        void GenerateIDESolution(std::string_view solutionPath) const;

        std::string GetIntermediateCodePath(const Runtime::ProjectInfo& projectInfo) const;
        std::string GetIDESolutionName(const Runtime::ProjectInfo& projectInfo) const;
        std::string GetIDESolutionPath(const Runtime::ProjectInfo& projectInfo) const;
    };
}