#include "GenerateFiles/GenerateEditorWindowRegister.h"
#include "GenerateFiles/GenerateComponentRegister.h"
#include "GenerateFiles/GenerateTypeDrawerReigster.h"
#include "GenerateFiles/GenerateResourceRegister.h"
#include "GenerateFiles/GenerateProjectInitialise.h"

#include "Utils.h"

#include <Reflect/Reflect.h>
#include <Reflect/CodeGenerate/CodeGenerate.h>

#include <fstream>
#include <ostream>
#include <string>
#include <filesystem>
#include <iostream>

namespace InsightReflectTool
{
    Reflect::Parser::FileParser ParseFilesInDirectory(const std::vector<std::string>& reflectDirectoies, 
                                                    const std::vector<std::string>& parserDirectoies, const Reflect::ReflectAddtionalOptions& options)
    {
        Reflect::Parser::FileParser parser;
        parser.SetIgnoreStrings(
            {
                "IS_CORE",
                "IS_MATHS",
                "IS_GRAPHICS",
                "IS_RUNTIME",
                "IS_EDITOR",
            });
        for (std::string_view dir : reflectDirectoies)
        {
            parser.ParseDirectory(dir.data(), &options);
        }
        for (std::string_view dir : parserDirectoies)
        {
            Reflect::Parser::FileParserOptions fileParserOptions = { };
            fileParserOptions.DoNotReflect = true;

            parser.ParseDirectory(dir.data(), &options, fileParserOptions);
        }
        Reflect::CodeGeneration::CodeGenerate codeGenerate;
        codeGenerate.Reflect(parser, &options);
        return parser;
    }
}

constexpr static const char* c_ArgType = "Type";
constexpr static const char* c_ArgParsePath = "ParsePath";
constexpr static const char* c_ArgReflectPath = "ReflectPath";
constexpr static const char* c_ArgGenerateProjectFileOutputPath = "GenerateProjectFileOutputPath";

int main(int argc, char** agc)
{
    using namespace InsightReflectTool;
    bool result = 0;

    std::unordered_map<std::string, std::vector<std::string>> arguments;
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

        arguments[key].push_back(value);
    }

    bool requiredArgsFound = false;
    requiredArgsFound |= arguments.find(c_ArgType) != arguments.end();
    requiredArgsFound |= arguments.find(c_ArgParsePath) != arguments.end();

    if (!requiredArgsFound)
    {
        std::cerr << "One of the required arguments was not found. Either 'Engine' or 'Project' must be given.";
        return -1;
    }

    auto iter = arguments.find(c_ArgGenerateProjectFileOutputPath);
    if (iter == arguments.end())
    {
        std::cerr << "'GenerateProjectFileOutputPath' argument must be given.";
        return -1;
    }

    auto typeIter = arguments.find(c_ArgType);
    std::string_view typeValue = *typeIter->second.begin();

    std::string genOutputPath = *arguments.find(c_ArgGenerateProjectFileOutputPath)->second.begin();

    std::string genEditorWindowsFile;
    std::string genComponentRegisterFile;
    std::string genTypeDrawerRegisterFile;

    if (genOutputPath.back() == '/' || genOutputPath.back() == '\\')
    {
        genOutputPath.pop_back();
    }

    if (typeValue == "Engine")
    {
        StateInfo::Instance().IsEngine = true;
        genEditorWindowsFile = genOutputPath + "/Editor/inc/EditorWindows.gen.h";
        genComponentRegisterFile = genOutputPath + "/Runtime/inc/ECS/RegisterComponents.gen.h";
        genTypeDrawerRegisterFile = genOutputPath + "/Editor/inc/TypeDrawers.gen.h";
    }
    else if (typeValue == "Project")
    {
        StateInfo::Instance().IsProject = true;
        genEditorWindowsFile = genOutputPath + "/EditorWindows.gen.h";
        genComponentRegisterFile = genOutputPath + "/RegisterComponents.gen.h";
        genTypeDrawerRegisterFile = genOutputPath + "/TypeDrawers.gen.h";
    }
    else
    {
        std::cerr << "Type argument is not valid. Value must be 'Engine' or 'Project'.";
        return -1;
    }

    Reflect::ReflectAddtionalOptions options;
    Reflect::Parser::FileParser fileParser = ParseFilesInDirectory(
        arguments[c_ArgReflectPath]
        , arguments[c_ArgParsePath]
        , options);

    GenerateEditorWindowRegister generateEditorWindowsRegister;
    result |= generateEditorWindowsRegister.Generate(fileParser, genEditorWindowsFile, options);

    GenerateComponentRegister generateComponentRegister;
    result |= generateComponentRegister.Generate(fileParser, genComponentRegisterFile, options);

    GenerateTypeDrawerReigster generateTypeDrawerReigster;
    result |= generateTypeDrawerReigster.Generate(fileParser, genTypeDrawerRegisterFile, options);

    if (typeValue == "Engine")
    {
        std::string genResourceRegisterFile;
        genResourceRegisterFile = genOutputPath + "/Runtime/inc/ResourceRegister.gen.h";

        GenerateResourceRegister generateResourceRegister;
        result |= generateResourceRegister.Generate(fileParser, genResourceRegisterFile, options);
    }
    else if (typeValue == "Project")
    {
        std::string projectInitialiseFileHeader = genOutputPath + "/ProjectInitialise.gen.h";
        std::string projectInitialiseFileSource = genOutputPath + "/ProjectInitialise.gen.cpp";

        GenerateProjectInitialise generateProjectInitialise;
        result |= generateProjectInitialise.GenerateHeader(fileParser, projectInitialiseFileHeader, options);
        result |= generateProjectInitialise.GenerateSource(fileParser, projectInitialiseFileSource, options);
    }

    return result;
}