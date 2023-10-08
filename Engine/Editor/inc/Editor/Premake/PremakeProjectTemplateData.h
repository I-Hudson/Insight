#pragma once

#include <vector>
#include <string>

namespace Insight::Editor
{
    struct PremakeProjectTemplateData
    {
        std::string ProjectName;
        std::string InsightRootPath;
        std::string PremakeOutputPath;

        std::vector<std::string> AdditionalFiles;
    
        static PremakeProjectTemplateData CreateFromProjectInfo();
    };

    typedef std::string(*CreatePremakeProjectTemplateFileFunc)(const PremakeProjectTemplateData&);
}