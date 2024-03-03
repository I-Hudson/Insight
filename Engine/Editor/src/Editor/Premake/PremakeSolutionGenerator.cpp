#include "Editor/Premake/PremakeSolutionGenerator.h"
#include "Editor/Premake/PremakeProjectTemplateData.h"

#include "FileSystem/FileSystem.h"

#include "Core/EnginePaths.h"
#include "Core/Logger.h"

namespace Insight::Editor
{
#if IS_DEBUG
#define BUILD_CONFIGURAION "Debug"
#elif IS_RELEASE
#define BUILD_CONFIGURAION "Release"
#else
//#error BUILD_CONFIGURAION must have a valid value.
#define BUILD_CONFIGURAION "UNKNOWN"
#endif

#if IS_PLATFORM_WINDOWS
#define BUILD_IDE "vs2022"
#define BUILD_PLATFORM "win64"
#define BUILD_SOULTION_EXTENSION ".sln"
#else
#error BUILD_IDE and BUILD_PLATFORM must have a valid value.
#endif

    bool PremakeSolutionGenerator::GenerateSolution(const PremakeTemplateData& templateData) const
    {
        if (!templateData.CreateFuncs)
        {
            return false;
        }

        RunInsightHeaderTool(templateData.HeaderToolData);
        GenerateProjectPremake(templateData.ProjectData, templateData.CreateFuncs.CreateProjectFunc);
        std::string solutionPremake = GenerateSolutionPremake(templateData.SolutionData, templateData.CreateFuncs.CreateSolutionFunc);
        GenerateIDESolution(solutionPremake);

        return true;
    }

    bool PremakeSolutionGenerator::BuildSolution(const char* solutionPath, const char* outputDirectory) const
    {
       std::string buildSolutionBatch = EnginePaths::GetInstallDirectory() + "/Build/Engine/Build_Solution.bat";

       std::string cmdCommend = "/c" + buildSolutionBatch + " " + solutionPath + BUILD_SOULTION_EXTENSION + " " + BUILD_IDE + " Build " + BUILD_CONFIGURAION + " " + BUILD_PLATFORM;
       return Platform::RunProcessAndWait(cmdCommend.c_str());
    }

    std::string PremakeSolutionGenerator::GetProjectIntermediateCodePath()
    {
        return Runtime::ProjectSystem::Instance().GetProjectInfo().GetIntermediatePath() + "/CodeFiles";
    }

    std::string PremakeSolutionGenerator::GetProjectIDESolutionName()
    {
        return Runtime::ProjectSystem::Instance().GetProjectInfo().ProjectName + "_Solution";
    }

    std::string PremakeSolutionGenerator::GetProjectIDESolutionPath()
    {
        return Runtime::ProjectSystem::Instance().GetProjectInfo().GetProjectPath() + "/" + GetProjectIDESolutionName();
    }

    void PremakeSolutionGenerator::RunInsightHeaderTool(const PremakeHeaderToolData& headerToolData) const
    {
        std::string insightRootPath = EnginePaths::GetInstallDirectory();

        for (size_t i = 0; i < headerToolData.ReflectDirectories.size(); ++i)
        {
            std::string reflectProjectCommand = "/c " + insightRootPath + "/bin/Release-windows-x86_64/InsightReflectTool/InsightReflectTool.exe";
            reflectProjectCommand += " Type=Project";
            reflectProjectCommand += " ParsePath=" + insightRootPath + "/Engine";
            reflectProjectCommand += " ReflectPath=" + headerToolData.ReflectDirectories.at(i);
            reflectProjectCommand += " GenerateProjectFileOutputPath=" + headerToolData.GeneratedFilesOutputPath;
            Platform::RunProcessAndWait(reflectProjectCommand.c_str());
        }
    }

    void PremakeSolutionGenerator::GenerateProjectPremake(const PremakeProjectTemplateData& projectData, const CreatePremakeProjectTemplateFileFunc& createProjectFunc) const
    {
        createProjectFunc(projectData);
    }

    std::string PremakeSolutionGenerator::GenerateSolutionPremake(const PremakeSolutionTemplateData& solutionData, const CreatePremakeSolutionTemplateFileFunc& createSoltuionFunc) const
    {
        return createSoltuionFunc(solutionData);
    }

    void PremakeSolutionGenerator::GenerateIDESolution(std::string_view solutionPath) const
    {
        if (solutionPath.empty() || !FileSystem::Exists(solutionPath))
        {
            IS_CORE_ERROR("[PremakeSolutionGenerator::GenerateIDESolution] Unable to generate solution as required premake file '{}' does not exist.", solutionPath);
            return;
        }

        std::string insightRootPath = EnginePaths::GetInstallDirectory();
        std::string generateProjectsBatch = "/c" + insightRootPath + "/Build/Engine/GENERATE_PROJECT.bat";
        std::string cmdCommend = generateProjectsBatch + " " + std::string(solutionPath) + " " + BUILD_IDE;
        Platform::RunProcessAndWait(cmdCommend.c_str());
    }
}