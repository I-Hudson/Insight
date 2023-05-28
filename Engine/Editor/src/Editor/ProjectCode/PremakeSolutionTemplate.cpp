#include "Editor/ProjectCode/PremakeSolutionTemplate.h"
#include "Editor/ProjectCode/PremakeProjectTemplate.h"

#include "FileSystem/FileSystem.h"

namespace Insight::Editor
{
    std::string CreatePremakeSolutionTemplateFile(const char* outFolder, const PremakeSolutionTemplateData& templateData)
    {
        std::string premake = c_PremakeSolutionTemplate;
        premake.replace(premake.find(c_PremakeSolutionTag_SolutionName), strlen(c_PremakeSolutionTag_SolutionName), templateData.SolutionName);
        premake.replace(premake.find(c_PremakeSolutionTag_ProjectName), strlen(c_PremakeSolutionTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeSolutionTag_InsightPath), strlen(c_PremakeSolutionTag_InsightPath), templateData.InsightRootPath);
        premake.replace(premake.find(c_PremakeSolutionTag_SolutionLocation), strlen(c_PremakeSolutionTag_SolutionLocation), "../../");
        premake.replace(premake.find(c_PremakeSolutionTag_PremakeProjectFileName), strlen(c_PremakeSolutionTag_PremakeProjectFileName), c_PremakeProjectFileName);

        std::string premakeOutFile = std::string(outFolder) + "/" + c_PremakeSolutionFileName;
        FileSystem::SaveToFile(premake, premakeOutFile, true);
        return premakeOutFile;
    }
}