#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace Insight::Editor
{
    struct PremakeProjectTemplateData
    {
        std::string ProjectName;
        std::string InsightRootPath;
        std::string PremakeOutputPath;

        std::vector<std::string> AdditionalFiles; // Add files to premake within the 'files' tag.
        std::unordered_map<std::string, std::vector<std::string>> VirtualFilePaths; // Create a new vpath in premake and add files to it.
    
        static PremakeProjectTemplateData CreateFromProjectInfo();
    };

    typedef std::string(*CreatePremakeProjectTemplateFileFunc)(const PremakeProjectTemplateData&);
}