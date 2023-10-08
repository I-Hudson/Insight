#include "Editor/Build/PakcageBuildPremakeProjectTemplate.h"

#include "FileSystem/FileSystem.h"

namespace Insight::Editor
{
    std::string CreatePackageBuildProjectFile(const PremakeProjectTemplateData& templateData)
    {
        std::string premake = c_PremakeProjectTemplate;

        std::string targetName = templateData.ProjectName;

        premake.replace(premake.find(c_PremakeProjectTag_ProjectName), strlen(c_PremakeProjectTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeProjectTag_InsightPath), strlen(c_PremakeProjectTag_InsightPath), templateData.InsightRootPath);
        premake.replace(premake.find(c_PremakeProjectTag_Targetname), strlen(c_PremakeProjectTag_Targetname), targetName);

        std::string premakeOutFile = std::string(templateData.PremakeOutputPath) + "/" + c_PremakeProjectFileName;
        std::vector<uint8_t> vec(premake.begin(), premake.end());
        FileSystem::SaveToFile(vec, premakeOutFile, FileType::Text, true);
        return premakeOutFile;
    }
}