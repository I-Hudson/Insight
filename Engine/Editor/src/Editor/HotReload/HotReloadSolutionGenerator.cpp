#include "Editor/HotReload/HotReloadSolutionGenerator.h"
#include "Editor/HotReload/HotReloadPremakeProjectTemplate.h"
#include "Editor/HotReload/HotReloadPremakeSolutionTemplate.h"

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
#error BUILD_CONFIGURAION must have a valid value.
#endif

#if IS_PLATFORM_WINDOWS
#define BUILD_IDE "vs2022"
#define BUILD_PLATFORM "win64"
#else
#error BUILD_IDE and BUILD_PLATFORM must have a valid value.
#endif

    bool HotReloadSolutionGenerator::GenerateSolution(const Runtime::ProjectInfo& projectInfo) const
    {
        RunInsightHeaderTool(projectInfo);
        GenerateProjectPremake(projectInfo);
        std::string solutionPremake = GenerateSolutionPremake(projectInfo);
        GenerateIDESolution(solutionPremake);

        return true;
    }

    bool HotReloadSolutionGenerator::BuildSolution(const Runtime::ProjectInfo& projectInfo) const
    {
       std::string buildSolutionBatch = EnginePaths::GetRootPath() + "/Build/Engine/Build_Solution.bat";

        std::string cmdCommend = buildSolutionBatch + " " + GetIDESolutionPath(projectInfo) + " " + BUILD_IDE + " Build " + BUILD_CONFIGURAION + " " + BUILD_PLATFORM;
        if (!cmdCommend.empty())
        {
            int buildErrorCode = std::system(cmdCommend.c_str());
            IS_CORE_INFO("[ProjectCodeSystem::MSBuildProject] Build error code: '{0}'.", buildErrorCode);
        }

        return true;
    }

    void HotReloadSolutionGenerator::RunInsightHeaderTool(const Runtime::ProjectInfo& projectInfo) const
    {
        std::string insightRootPath = EnginePaths::GetRootPath();

        std::string reflectProjectCommand = "" + insightRootPath + "/bin/Release-windows-x86_64/InsightReflectTool/InsightReflectTool.exe";
        reflectProjectCommand += " Type=Project";
        reflectProjectCommand += " ParsePath=" + projectInfo.GetContentPath();
        reflectProjectCommand += " GenerateProjectFileOutputPath=" + GetIntermediateCodePath(projectInfo) + "/Generated";
        std::system(reflectProjectCommand.c_str());
    }

    void HotReloadSolutionGenerator::GenerateProjectPremake(const Runtime::ProjectInfo& projectInfo) const
    {
        std::string insightRootPath = EnginePaths::GetRootPath();

        PremakeProjectTemplateData projectTemplateData;
        projectTemplateData.ProjectName = projectInfo.ProjectName.c_str();
        projectTemplateData.InsightRootPath = insightRootPath.c_str();
        projectTemplateData.AdditionalFiles.push_back(GetIntermediateCodePath(projectInfo) + "/Generated");

        CreatePremakeProjectTemplateFile(GetIntermediateCodePath(projectInfo).c_str(), projectTemplateData);
    }

    std::string HotReloadSolutionGenerator::GenerateSolutionPremake(const Runtime::ProjectInfo& projectInfo) const
    {
        std::string insightRootPath = EnginePaths::GetRootPath();
        std::string solutionName = GetIDESolutionName(projectInfo);

        PremakeSolutionTemplateData solutionTemplateData;
        solutionTemplateData.SolutionName = solutionName.c_str();
        solutionTemplateData.ProjectName = projectInfo.ProjectName.c_str();
        solutionTemplateData.InsightRootPath = insightRootPath.c_str();

        return CreatePremakeSolutionTemplateFile(GetIntermediateCodePath(projectInfo).c_str(), solutionTemplateData);
    }

    void HotReloadSolutionGenerator::GenerateIDESolution(std::string_view solutionPath) const
    {
        if (solutionPath.empty() || !FileSystem::Exists(solutionPath))
        {
            IS_CORE_ERROR("[HotReloadSolutionGenerator::GenerateIDESolution] Unable to generate solution as required premake file '{}' does not exist.", solutionPath);
            return;
        }

        std::string insightRootPath = EnginePaths::GetRootPath();
        std::string generateProjectsBatch = insightRootPath + "/Build/Engine/GENERATE_PROJECT.bat";
        std::string cmdCommend = generateProjectsBatch + " " + std::string(solutionPath) + " " + BUILD_IDE;
        std::system(cmdCommend.c_str());
    }

    std::string HotReloadSolutionGenerator::GetIntermediateCodePath(const Runtime::ProjectInfo& projectInfo) const
    {
        return projectInfo.GetIntermediatePath() + "/CodeFiles";
    }

    std::string HotReloadSolutionGenerator::GetIDESolutionName(const Runtime::ProjectInfo& projectInfo) const
    {
        return projectInfo.ProjectName + "_Solution";
    }

    std::string HotReloadSolutionGenerator::GetIDESolutionPath(const Runtime::ProjectInfo& projectInfo) const
    {
        return projectInfo.GetProjectPath() + "/" + GetIDESolutionName(projectInfo) + ".sln";
    }
}