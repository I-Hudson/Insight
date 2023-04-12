#include "GenerateEditorWindowRegister.h"

#include <Reflect.h>
#include <CodeGenerate/CodeGenerate.h>

#include <fstream>
#include <ostream>
#include <string>
#include <filesystem>


std::string engineRootDir = "../../../Engine";

int main(int argc, char** agc)
{
    bool result = 0;

    GenerateEditorWindowRegister generateEditorWindowsRegister;
    result |= generateEditorWindowsRegister.Generate(engineRootDir + "/Editor", engineRootDir + "/Editor/inc/EditorWindows.gen.h");

    return result;
}