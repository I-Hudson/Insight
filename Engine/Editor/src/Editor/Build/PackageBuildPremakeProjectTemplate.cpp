#include "Editor/Build/PakcageBuildPremakeProjectTemplate.h"

#include "Core/EnginePaths.h"
#include "FileSystem/FileSystem.h"

#include <filesystem>

namespace Insight::Editor
{
    std::string CreatePackageBuildProjectFile(const PremakeProjectTemplateData& templateData)
    {
        std::string premake = c_PremakeProjectTemplate;

        std::string targetName = templateData.ProjectName;

        premake.replace(premake.find(c_PremakeProjectTag_ProjectName), strlen(c_PremakeProjectTag_ProjectName), templateData.ProjectName);
        premake.replace(premake.find(c_PremakeProjectTag_InsightPath), strlen(c_PremakeProjectTag_InsightPath), templateData.InsightRootPath);
        premake.replace(premake.find(c_PremakeProjectTag_Targetname), strlen(c_PremakeProjectTag_Targetname), targetName);

        std::string dllPaths;
#ifdef IS_DEBUG
        const char* buildConfiguration = "Debug";
#elif defined(IS_RELEASE)
        const char* buildConfiguration = "Release";
#else
        const char* buildConfiguration = "UNKNOWN";
#endif
        const std::string srcDLLFolder = EnginePaths::GetRootPath() + "/deps/" + buildConfiguration + "-windows-x86_64/dll";

        for (const auto& path : std::filesystem::recursive_directory_iterator(srcDLLFolder))
        {
            std::string formatedPath = path.path().string();
            FileSystem::PathToUnix(formatedPath);

            std::string relativePath = formatedPath;
            relativePath.replace(formatedPath.find(srcDLLFolder), srcDLLFolder.size(), "");

            if (path.is_regular_file()
                && path.path().filename().string().find("Insight_") == std::string::npos)
            {
               //dllPaths += "\"{COPY} \\\"" + formatedPath + "\\\" \\\"%{cfg.targetdir}" + relativePath + "\\\"\",\n";
            }
        }
        premake.replace(premake.find(c_PremakeProjectTag_PrebuildCommands), strlen(c_PremakeProjectTag_PrebuildCommands), dllPaths);

        std::string premakeOutFile = std::string(templateData.PremakeOutputPath) + "/" + c_PremakeProjectFileName;
        std::vector<uint8_t> vec(premake.begin(), premake.end());
        FileSystem::SaveToFile(vec, premakeOutFile, FileType::Text, true);
        return premakeOutFile;
    }
}