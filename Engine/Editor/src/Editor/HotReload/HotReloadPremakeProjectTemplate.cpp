#include "Editor/HotReload/HotReloadPremakeProjectTemplate.h"

#include "FileSystem/FileSystem.h"

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

namespace Insight::Editor
{
    std::string CreatePremakeProjectTemplateFile(const PremakeProjectTemplateData& templateData)
    {
        std::string premake = c_PremakeProjectTemplate;

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H-%M-%S");
        std::string timeAndDate = ss.str();

        std::string targetName = templateData.ProjectName; 
        targetName += "_" + timeAndDate;

        premake.replace(premake.find(c_PremakeProjectTag_ProjectName), strlen(c_PremakeProjectTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeProjectTag_InsightPath), strlen(c_PremakeProjectTag_InsightPath), templateData.InsightRootPath);
        premake.replace(premake.find(c_PremakeProjectTag_Targetname), strlen(c_PremakeProjectTag_Targetname), targetName);

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

        std::string virtualPaths;
        for (const auto& [tag, paths] : templateData.VirtualFilePaths)
        {
            virtualPaths += "[\"" + tag + "\"] = { ";
            for (const std::string& path : paths)
            {
                virtualPaths += "\"" + path + "\",\n";

            }
            virtualPaths += "},\n";
        }
        premake.replace(premake.find(c_PremakeProjectTag_VirtualPaths), strlen(c_PremakeProjectTag_VirtualPaths), virtualPaths);

        std::string premakeOutFile = std::string(templateData.PremakeOutputPath) + "/" + c_PremakeProjectFileName;
        std::vector<uint8_t> vec(premake.begin(), premake.end());
        FileSystem::SaveToFile(vec, premakeOutFile, FileType::Text, true);
        return premakeOutFile;
    }
}