#include "Editor/ProjectCode/PremakeProjectTemplate.h"

#include "FileSystem/FileSystem.h"


namespace Insight::Editor
{
    std::string CreatePremakeProjectTemplateFile(const char* outFolder, const PremakeProjectTemplateData& templateData)
    {
        std::string premake = c_PremakeProjectTemplate;
        premake.replace(premake.find(c_PremakeProjectTag_ProjectName), strlen(c_PremakeProjectTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeProjectTag_InsightPath), strlen(c_PremakeProjectTag_InsightPath), templateData.InsightRootPath);

        std::string premakeOutFile = std::string(outFolder) + "/" + c_PremakeProjectFileName;
        FileSystem::SaveToFile(premake, premakeOutFile, true);
        return premakeOutFile;
    }
}