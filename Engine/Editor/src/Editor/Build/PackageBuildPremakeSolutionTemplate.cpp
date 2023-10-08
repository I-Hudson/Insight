#include "Editor/Build/PackageBuildPremakeSolutionTemplate.h"

#include "FileSystem/FileSystem.h"

namespace Insight::Editor
{
    std::string CreatePackageBuildSolutionFile(const PremakeSolutionTemplateData& templateData)
    {
        std::string premake = c_PremakeSolutionTemplate;
        premake.replace(premake.find(c_PremakeSolutionTag_SolutionName), strlen(c_PremakeSolutionTag_SolutionName), templateData.SolutionName);
        premake.replace(premake.find(c_PremakeSolutionTag_ProjectName), strlen(c_PremakeSolutionTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeSolutionTag_InsightPath), strlen(c_PremakeSolutionTag_InsightPath), templateData.InsightRootPath);
        premake.replace(premake.find(c_PremakeSolutionTag_SolutionLocation), strlen(c_PremakeSolutionTag_SolutionLocation), "./");

        std::string premakeOutFile = templateData.PremakeOutputPath + "/" + c_PremakeSolutionFileName;
        std::vector<uint8_t> vec(premake.begin(), premake.end());
        FileSystem::SaveToFile(vec, premakeOutFile, FileType::Text, true);
        return premakeOutFile;
    }
}