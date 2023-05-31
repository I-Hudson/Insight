#include "GenerateFiles/GenerateEditorWindowRegister.h"

#include "Utils.h"

#include <CodeGenerate/CodeGenerate.h>

#include <string>
#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateEditorWindowRegister::Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> editorWindowFiles;
        std::vector<Reflect::ReflectContainerData> editorWindowClasses;
        for (const auto& fileParsed : fileParser.GetAllFileParsedData())
        {
            for (const auto& reflectData : fileParsed.ReflectData)
            {
                if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [](const Reflect::ReflectInheritanceData& data)
                    {
                        return "IEditorWindow" == data.Name;
                    }) != reflectData.Inheritance.end())
                {
                    editorWindowFiles.push_back(fileParsed.FilePath + "/" + fileParsed.FileName + ".h");
                    editorWindowClasses.push_back(reflectData);
                }
            }
        }

        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);
            std::string fileOutputAbsPath = std::filesystem::canonical(outFilePath).parent_path().generic_string();

            file << "#include \"Editor/EditorWindows/EditorWindowManager.h\"" << NEW_LINE;
            Utils::WriteIncludeFiles(file, fileOutputAbsPath, editorWindowFiles);

            file << NEW_LINE;

            file << "namespace Insight {\n";
            TAB_N(1);
            file << "namespace Editor {\n";

            Utils::WriteSourceFunctionDefinition(file, "void", "RegisterAllEditorWindows", {}, [&](std::fstream& file)
                {
                    for (const Reflect::ReflectContainerData& reflectData : editorWindowClasses)
                    {
                        TAB_N(3);
                        file << "EditorWindowManager::Instance().RegisterEditorWindow<::" + reflectData.NameWithNamespace + ">();" << NEW_LINE;
                    }
                }, 2);

            Utils::WriteSourceFunctionDefinition(file, "void", "UnregisterAllEditorWindows", {}, [&](std::fstream& file)
                {
                    for (const Reflect::ReflectContainerData& reflectData : editorWindowClasses)
                    {
                        TAB_N(3);
                        file << "EditorWindowManager::Instance().UnregisterEditorWindow<::" + reflectData.NameWithNamespace + ">();" << NEW_LINE;
                    }
                }, 2);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", "GetAllEditorWindowNames", {}, [&](std::fstream& file)
                {
                    TAB_N(3);
                    file << "std::vector<std::string> editorWindowNames;" << NEW_LINE;
                    for (const Reflect::ReflectContainerData& reflectData : editorWindowClasses)
                    {
                        TAB_N(3);
                        file << "editorWindowNames.push_back(::" + reflectData.NameWithNamespace + "::WINDOW_NAME);" << NEW_LINE;
                    }
                    TAB_N(3);
                    file << "return editorWindowNames;" << NEW_LINE;
                }, 2);

            TAB_N(1);
            file << "}" << NEW_LINE;
            file << "}" << NEW_LINE;
            file.close();

            return true;
        }
        else
        {
            return false;
        }
    }
}