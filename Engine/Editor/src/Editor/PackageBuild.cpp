#include "Editor/PackageBuild.h"
#include "Editor/Premake/PremakeSolutionGenerator.h"

#include "Editor/Build/PackageBuildPremakeSolutionTemplate.h"
#include "Editor/Build/PakcageBuildPremakeProjectTemplate.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Core/EnginePaths.h"

#include "Runtime/ProjectSystem.h"
#include "Runtime/RuntimeSettings.h"

#include "Asset/AssetPackage/AssetPackageZip.h"
#include "Serialisation/Serialisers/JsonSerialiser.h"

#include "Platforms/Platform.h"
#include "Editor/Editor.h"

#include <filesystem>

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_generateProjectBach = "/../../../Build/Engine/GENERATE_PROJECT.bat";

        void PackageBuild::Build(std::string_view outputFolder, const PackageBuildOptions& options)
        {
            PlatformProgress progressBar;
            progressBar.Show("Build Progress");

            progressBar.UpdateProgress(0, "Removing all old files");
            for (const auto& entry : std::filesystem::directory_iterator(outputFolder))
            {
                std::filesystem::remove_all(entry.path());
            }

            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

            PremakeHeaderToolData headerToolData;
            headerToolData.ReflectDirectories.push_back(Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath());
            headerToolData.GeneratedFilesOutputPath = PremakeSolutionGenerator::GetProjectIntermediateCodePath() + "/Generated";

            PremakeTemplateData templateData;
            templateData.HeaderToolData = std::move(headerToolData);
            templateData.SolutionData = PremakeSolutionTemplateData::CreateFromProjectInfo(PremakeSolutionGenerator::GetProjectIDESolutionName().c_str());
            templateData.SolutionData.PremakeOutputPath = projectInfo.GetIntermediatePath() + "/PackageBuild";

            templateData.ProjectData = PremakeProjectTemplateData::CreateFromProjectInfo();
            templateData.ProjectData.AdditionalFiles.push_back(templateData.HeaderToolData.GeneratedFilesOutputPath);
            templateData.ProjectData.PremakeOutputPath = projectInfo.GetIntermediatePath() + "/PackageBuild";

            if (options.EnableProfiling)
            {
                templateData.ProjectData.AdditionalDefines.push_back("IS_PROFILE_ENABLED");
                templateData.ProjectData.AdditionalDefines.push_back("IS_PROFILE_TRACY");
                templateData.ProjectData.AdditionalDefines.push_back("TRACY_IMPORTS");
                templateData.ProjectData.AdditionalDefines.push_back("TRACY_ON_DEMAND");
            }

            templateData.ProjectData.AdditionalDefines.push_back("IS_PACKAGE_BUILD");

            templateData.CreateFuncs.CreateSolutionFunc = CreatePackageBuildSolutionFile;
            templateData.CreateFuncs.CreateProjectFunc = CreatePackageBuildProjectFile;

            progressBar.UpdateProgress(33, "Solution being generated");
            PremakeSolutionGenerator solutionGenerator;
            solutionGenerator.GenerateSolution(templateData);

            progressBar.UpdateProgress(60, "Building solution");
            std::string solutionPath = projectInfo.GetIntermediatePath() + "/PackageBuild/" + PremakeSolutionGenerator::GetProjectIDESolutionName();
            solutionGenerator.BuildSolution(solutionPath.c_str(), outputFolder.data());

            progressBar.UpdateProgress(85, "Copying built exe to output folder");
#ifdef IS_DEBUG
            const char* buildConfiguration = "Debug";
#elif defined(IS_RELEASE)
            const char* buildConfiguration = "Release";
#else
            const char* buildConfiguration = "UNKNOWN";

#endif
            const std::string buildExeFolder = projectInfo.GetIntermediatePath() + "/PackageBuild/bin/" + buildConfiguration + "-windows-x86_64/" + projectInfo.ProjectName;
            std::filesystem::copy(buildExeFolder, outputFolder, std::filesystem::copy_options::recursive);

            progressBar.UpdateProgress(90, "Copying engine resources to output folder");
            CopyEngineResourceFiles(outputFolder);

            progressBar.UpdateProgress(95, "Building runtime settings");
            BuildRuntimeSettings(outputFolder);
            progressBar.UpdateProgress(97, "Building project assets");
            BuildContentFiles(outputFolder);

            //BuildSolution();
            //BuildPackageBuild(outputFolder);
        }

        void PackageBuild::BuildSolution()
        {
            std::string exeFillPath = Platform::GetExecuteablePath();
            std::string premakePath = exeFillPath + "/../../../vendor/premake/premake5.exe";
            std::string solutionLuaPath = exeFillPath + GenerateBuildFile();

            std::string command = "/c ";
            command += premakePath;
            command += " --file=";
            command += solutionLuaPath;
            command += " vs2019";

            Platform::RunProcessAndWait(command.c_str());
        }

        void PackageBuild::BuildPackageBuild(std::string_view outputFolder)
        {
            std::string exeFillPath = Platform::GetExecuteablePath();
            std::string buildSolutionBatch = exeFillPath + "/../../../Build/Engine/Build_Solution.bat";
            std::string solution = exeFillPath + "/../../../InsightStandalone.sln";

            std::string command = "/c ";
            command += buildSolutionBatch;
            command += " ";
            command += solution;
            command += " vs2019 Build Debug win64 ";
            command += outputFolder;

            Platform::RunProcessAndWait(command.c_str());
        }

        std::string PackageBuild::GenerateBuildFile()
        {
            std::vector<Byte> fileData = FileSystem::ReadFromFile("");
            std::string fileString{ fileData.begin(), fileData.end() };
            fileData.resize(0);

            const char* c_ProjectFilesToeken = "--PROJECT_FILES";
            u64 projectFilesToken = fileString.find(c_ProjectFilesToeken);
            if (projectFilesToken == std::string::npos)
            {
                return "";
            }

            std::string projectFiles = "";
            fileString.replace(projectFilesToken, strlen(c_ProjectFilesToeken), projectFiles);

            return fileString;
        }

        void PackageBuild::CopyEngineResourceFiles(std::string_view outputFolder) const
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            std::filesystem::copy(EnginePaths::GetResourcePath(), std::string(outputFolder) + "/Resources", std::filesystem::copy_options::recursive);
        }

        void PackageBuild::BuildRuntimeSettings(std::string_view outputFolder) const
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

            const std::string runtimeSettingsPath = projectInfo.GetIntermediatePath() + "/PackageBuild/BuiltContent/RuntimeSettings.json";

            Serialisation::JsonSerialiser serialsier(false);
            Runtime::RuntimeSettings::Instance().Serialise(&serialsier);
            FileSystem::SaveToFile(serialsier.GetSerialisedData(), runtimeSettingsPath, true);

            Runtime::AssetPackageZip builtContent(projectInfo.GetIntermediatePath() + "/PackageBuild/BuiltContent", "BuiltContent");
            Runtime::AssetRegistry::Instance().AddAsset(runtimeSettingsPath, &builtContent, false, false);

            const std::string biultContentPath = projectInfo.GetIntermediatePath() + "/PackageBuild/BuiltContent/BuiltContent.zip";
            builtContent.BuildPackage(biultContentPath);

            std::filesystem::copy(biultContentPath, outputFolder, std::filesystem::copy_options::update_existing);
        }

        void PackageBuild::BuildContentFiles(std::string_view outputFolder) const
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            const std::string projectAssetsPath = projectInfo.GetContentPath() + "/ProjectAssets.zip";
            Runtime::AssetRegistry::Instance().GetAssetPackageFromName(Editor::Editor::c_ProjectAssetPackageName)->BuildPackage(projectAssetsPath);
            std::filesystem::copy(projectAssetsPath, outputFolder, std::filesystem::copy_options::update_existing);
            std::filesystem::remove(projectAssetsPath);
        }
    }
}