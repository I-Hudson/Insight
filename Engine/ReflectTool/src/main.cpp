#include "GenerateFiles/GenerateEditorWindowRegister.h"
#include "GenerateFiles/GenerateComponentRegister.h"
#include "GenerateFiles/GenerateProjectInitialise.h"

#include <Reflect.h>
#include <CodeGenerate/CodeGenerate.h>

#include <fstream>
#include <ostream>
#include <string>
#include <filesystem>

namespace InsightReflectTool
{
    Reflect::FileParser ParseFilesInDirectory(std::string_view directory, const Reflect::ReflectAddtionalOptions& options)
    {
        Reflect::FileParser parser;
        parser.SetIgnoreStrings(
            {
                "IS_CORE",
                "IS_MATHS",
                "IS_GRAPHICS",
                "IS_RUNTIME",
                "IS_EDITOR",
            });
        parser.ParseDirectory(directory.data(), &options);
        Reflect::CodeGenerate codeGenerate;
        codeGenerate.Reflect(parser, &options);
        return parser;
    }
}

constexpr static const char* c_ArgType = "Type";
constexpr static const char* c_ArgParsePath = "ParsePath";
constexpr static const char* c_ArgGenerateProjectFileOutputPath = "GenerateProjectFileOutputPath";

int main(int argc, char** agc)
{
    using namespace InsightReflectTool;
    bool result = 0;

    std::unordered_map<std::string, std::string> arguments;
    for (size_t i = 0; i < argc; ++i)
    {
        std::string arg = agc[i];
        uint64_t splitChar = arg.find('=');
        if (splitChar == std::string::npos)
        {
            continue;
        }
        std::string key = arg.substr(0, splitChar);
        std::string value = arg.substr(splitChar + 1);

        arguments[key] = value;
    }

    bool requiredArgsFound = false;
    requiredArgsFound |= arguments.find(c_ArgType) != arguments.end();
    requiredArgsFound |= arguments.find(c_ArgParsePath) != arguments.end();

    if (!requiredArgsFound)
    {
        std::cerr << "One of the required arguments was not found. Either 'Engine' or 'Project' must be given.";
        std::cin;
        return 0;
    }

    auto typeIter = arguments.find(c_ArgType);
    std::string_view typeValue = typeIter->second;

    std::string rootPath = arguments.find(c_ArgParsePath)->second; //"../../../Engine/";
    std::string genEditorWindowsFile;
    std::string genComponentRegisterFile;

    std::string projectInitialiseFile;

    if (rootPath.back() == '/' || rootPath.back() == '\\')
    {
        rootPath.pop_back();
    }

    if (typeValue == "Engine")
    {
        genEditorWindowsFile = rootPath + "/Editor/inc/EditorWindows.gen.h";
        genComponentRegisterFile = rootPath + "/Runtime/inc/ECS/RegisterComponents.gen.h";
    }
    else if (typeValue == "Project")
    {
        auto iter = arguments.find(c_ArgGenerateProjectFileOutputPath);
        if (iter == arguments.end())
        {
            std::cerr << "'Project' type must have a 'GenerateProjectFileOutputPath' argument.";
            std::cin;
            return 0;
        }

        std::string genOutputPath = iter->second;
        genEditorWindowsFile = genOutputPath + "/EditorWindows.gen.h";
        genComponentRegisterFile = genOutputPath + "/RegisterComponents.gen.h";
        projectInitialiseFile = genOutputPath + "/ProjectInitialise.gen.cpp";
    }
    else
    {
        std::cerr << "Type argument is not valid. Value must be 'Engine' or 'Project'.";
        std::cin;
        return 0;
    }

    Reflect::ReflectAddtionalOptions options;
    Reflect::FileParser fileParser = ParseFilesInDirectory(rootPath, options);

    GenerateEditorWindowRegister generateEditorWindowsRegister;
    result |= generateEditorWindowsRegister.Generate(fileParser, genEditorWindowsFile, options);

    GenerateComponentRegister generateComponentRegister;
    result |= generateComponentRegister.Generate(fileParser, genComponentRegisterFile, options);

    if (typeValue == "Project")
    {
        GenerateProjectInitialise generateProjectInitialise;
        result |= generateProjectInitialise.Generate(fileParser, projectInitialiseFile, options);
    }

    std::cin;

    return result;
}