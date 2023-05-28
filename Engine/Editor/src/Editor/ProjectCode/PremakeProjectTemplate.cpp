#include "Editor/ProjectCode/PremakeProjectTemplate.h"

#include "FileSystem/FileSystem.h"


namespace Insight::Editor
{
    std::string CreatePremakeProjectTemplateFile(const char* outFolder, const PremakeProjectTemplateData& templateData)
    {
        std::string premake = c_PremakeProjectTemplate;
        premake.replace(premake.find(c_PremakeProjectTag_ProjectName), strlen(c_PremakeProjectTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeProjectTag_InsightPath), strlen(c_PremakeProjectTag_InsightPath), templateData.InsightRootPath);

        std::string additionalFiles;
        for (std::string view : templateData.AdditionalFiles)
        {
            additionalFiles += "\"" + view + "/**.hpp" + "\", \n";
            additionalFiles += "\"" + view + "/**.h" + "\", \n";
            additionalFiles += "\"" + view + "/**.inl" + "\", \n";
            additionalFiles += "\"" + view + "/**.cpp" + "\", \n";
            additionalFiles += "\"" + view + "/**.c" + "\", \n";
        }
        premake.replace(premake.find(c_PremakeProjectTag_AdditionalFiles), strlen(c_PremakeProjectTag_AdditionalFiles), additionalFiles);


        std::string premakeOutFile = std::string(outFolder) + "/" + c_PremakeProjectFileName;
        FileSystem::SaveToFile(premake, premakeOutFile, true);
        return premakeOutFile;
    }
}