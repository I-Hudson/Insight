#include "Editor/PackageBuild.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"
#include "Threading/TaskSystem.h"

#include <processthreadsapi.h>
#include <shellapi.h>

namespace Insight
{
    namespace Editor
    {
        constexpr const char* c_generateProjectBach = "/../../../Build/Engine/GENERATE_PROJECT.bat";
        constexpr const char* c_packageBuildPremakeScript = "/../../../Build/Engine/premake5-standalone.lua";

        void PackageBuild::Build(std::string_view outputFolder)
        {
            BuildSolution();
            BuildPackageBuild(outputFolder);
        }

        std::string PackageBuild::GetExecuteablepath()
        {
            char path[MAX_PATH];
            GetModuleFileNameA(NULL, path, MAX_PATH);
            std::string sString(path);
            sString = FileSystem::FileSystem::PathToUnix(sString);
            sString = sString.substr(0, sString.find_last_of('/'));
            return sString;
        }

        void PackageBuild::BuildSolution()
        {
            std::string exeFillPath = GetExecuteablepath();
            std::string premakePath = exeFillPath + "/../../../vendor/premake/premake5.exe";
            std::string solutionLuaPath = exeFillPath + c_packageBuildPremakeScript;

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
    }
}