#include <Reflect.h>

#include <fstream>
#include <ostream>
#include <string>
#include <filesystem>

std::string engineRootDir = "../../../Engine";


Reflect::FileParser ParseFilesInDirectory(std::string directory)
{
    Reflect::FileParser parser;
    Reflect::ReflectAddtionalOptions options = { };
    parser.ParseDirectory(directory, &options);
    Reflect::CodeGenerate codeGenerate;
    codeGenerate.Reflect(parser, &options);
    return parser;
}

void FindingAllEditorWindowClasses()
{
    std::string editorIncFolder = engineRootDir + "/Editor/inc";
    Reflect::FileParser parser = ParseFilesInDirectory(editorIncFolder);

    std::vector<std::string> editorWindowClasses;
    for (const auto& fileParsed : parser.GetAllFileParsedData())
    {
        for (const auto& reflectData : fileParsed.ReflectData)
        {
            if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [](const Reflect::ReflectInheritanceData& data)
                {
                    return "IEditorWindow" == data.Name;
                }) != reflectData.Inheritance.end())
            {
                editorWindowClasses.push_back(reflectData.Name);
            }
        }
    }

    std::fstream file;
    std::string absPath = std::filesystem::absolute(editorIncFolder + "/EditorWindows.gen.h").string();
    file.open(absPath, std::ios::out | std::ios::trunc);
    if (file.is_open())
    {
        file << "#pragma once" << NEW_LINE;

        file << "#include \"Editor/EditorWindows/EditorWindowManager.h\"" << NEW_LINE;
        for (const std::string& str : editorWindowClasses)
        {
            file << "#include \"Editor/EditorWindows/" + str + ".h\"" << NEW_LINE;
        }

        file << "namespace Insight {\n";
        TAB_N(1);
        file << "namespace Editor {\n";

        TAB_N(2);
        file << "void RegisterAllEditorWindows()" << NEW_LINE;
        TAB_N(2);
        file << "{" << NEW_LINE;

        for (const std::string& str : editorWindowClasses)
        {
            TAB_N(3);
            file << "EditorWindowManager::Instance().RegsiterEditorWindow<" + str + ">();" << NEW_LINE;
        }

        TAB_N(2);
        file << "}" << NEW_LINE;
        TAB_N(1);
        file << "}" << NEW_LINE;
        file << "}" << NEW_LINE;
        file.close();
    }
}

int main(int argc, char** agc)
{
    FindingAllEditorWindowClasses();

    return 0;
}