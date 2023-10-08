#pragma once

#include <vector>
#include <string>

namespace Insight::Editor
{
    struct PremakeSolutionTemplateData
    {
        std::string SolutionName;
        std::string ProjectName;
        std::string InsightRootPath;
        std::string PremakeOutputPath;

        static PremakeSolutionTemplateData CreateFromProjectInfo(const char* solutonName);
    };

    typedef std::string(*CreatePremakeSolutionTemplateFileFunc)(const PremakeSolutionTemplateData&);

}