#include "Editor/PackageBuild.h"
#include "Editor/Premake/PremakeSolutionGenerator.h"

#include "Editor/Build/PackageBuildPremakeSolutionTemplate.h"
#include "Editor/Build/PakcageBuildPremakeProjectTemplate.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"

#include "Runtime/ProjectSystem.h"

#include <processthreadsapi.h>
#include <shellapi.h>

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_generateProjectBach = "/../../../Build/Engine/GENERATE_PROJECT.bat";

        void PackageBuild::Build(std::string_view outputFolder)
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

            PremakeHeaderToolData headerToolData;
            headerToolData.ReflectDirectories.push_back(Runtime::ProjectSystem::Instance().GetProjectInfo().GetProjectPath());
            headerToolData.GeneratedFilesOutputPath = PremakeSolutionGenerator::GetProjectIntermediateCodePath() + "/Generated";

            PremakeTemplateData templateData;
            templateData.HeaderToolData = std::move(headerToolData);
            templateData.SolutionData = PremakeSolutionTemplateData::CreateFromProjectInfo(PremakeSolutionGenerator::GetProjectIDESolutionName().c_str());
            templateData.SolutionData.PremakeOutputPath = projectInfo.GetIntermediatePath() + "/PackageBuild";

            templateData.ProjectData = PremakeProjectTemplateData::CreateFromProjectInfo();
            templateData.ProjectData.AdditionalFiles.push_back(templateData.HeaderToolData.GeneratedFilesOutputPath);
            templateData.ProjectData.PremakeOutputPath = projectInfo.GetIntermediatePath() + "/PackageBuild";

            templateData.CreateFuncs.CreateSolutionFunc = CreatePackageBuildSolutionFile;
            templateData.CreateFuncs.CreateProjectFunc = CreatePackageBuildProjectFile;

            PremakeSolutionGenerator solutionGenerator;
            solutionGenerator.GenerateSolution(templateData);

            //BuildSolution();
            //BuildPackageBuild(outputFolder);
        }

        std::string PackageBuild::GetExecuteablepath()
        {
            char path[MAX_PATH];
            GetModuleFileNameA(NULL, path, MAX_PATH);
            std::string sString(path);
            FileSystem::PathToUnix(sString);
            sString = sString.substr(0, sString.find_last_of('/'));
            return sString;
        }

        void PackageBuild::BuildSolution()
        {
            std::string exeFillPath = GetExecuteablepath();
            std::string premakePath = exeFillPath + "/../../../vendor/premake/premake5.exe";
            std::string solutionLuaPath = exeFillPath + GenerateBuildFile();

            std::string command = "/c ";
            command += premakePath;
            command += " --file=";
            command += solutionLuaPath;
            command += " vs2019";

            STARTUPINFOA info = { sizeof(info) };
            PROCESS_INFORMATION processInfo;
            IS_CORE_INFO("Command line: '{}'.", command);
            if (!CreateProcessA("C:\\Windows\\System32\\cmd.exe", (char*)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
            {
                IS_CORE_ERROR("[PackageBuild::BuildSolution] Unable to compile solution: '{0}'.", GetLastError());
            }
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
        }

        void PackageBuild::BuildPackageBuild(std::string_view outputFolder)
        {
            std::string exeFillPath = GetExecuteablepath();
            std::string buildSolutionBatch = exeFillPath + "/../../../Build/Engine/Build_Solution.bat";
            std::string solution = exeFillPath + "/../../../InsightStandalone.sln";

            std::string command = "/c ";
            command += buildSolutionBatch;
            command += " ";
            command += solution;
            command += " vs2019 Build Debug win64 ";
            command += outputFolder;

            STARTUPINFOA info = { sizeof(info) };
            PROCESS_INFORMATION processInfo;
            if (!CreateProcessA("C:\\Windows\\System32\\cmd.exe", (char*)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
            {
                IS_CORE_ERROR("[PackageBuild::BuildPackageBuild] Unable to compile solution: '{0}'.", GetLastError());
            }
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
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
    }
}