#include "GenerateFiles/GenerateEditorWindowRegister.h"
#include "GenerateFiles/GenerateComponentRegister.h"

#include <Reflect.h>
#include <CodeGenerate/CodeGenerate.h>

#include <fstream>
#include <ostream>
#include <string>
#include <filesystem>


std::string engineRootDir = "../../../Engine";

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

int main(int argc, char** agc)
{
    using namespace InsightReflectTool;
    bool result = 0;

    Reflect::ReflectAddtionalOptions options;
    Reflect::FileParser fileParser = ParseFilesInDirectory(engineRootDir, options);

    GenerateEditorWindowRegister generateEditorWindowsRegister;
    result |= generateEditorWindowsRegister.Generate(fileParser, engineRootDir + "/Editor/inc/EditorWindows.gen.h", options);

    GenerateComponentRegister generateComponentRegister;
    result |= generateComponentRegister.Generate(fileParser, engineRootDir + "Runtime/inc/Components.gen.h", options);

    return result;
}