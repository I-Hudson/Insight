#pragma once

#include "WriteFileHeader.h"
#include "FileParser.h"

#include <CodeGenerate/CodeGenerate.h>

#include <string>
#include <filesystem>

class GenerateEditorWindowRegister
{
public:

    bool Generate(std::string_view folderPath, std::string_view outFilePath)
    {
        Reflect::FileParser parser = ParseFilesInDirectory(folderPath);

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
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            WriteFileHeader(file);

            file << "#include \"Editor/EditorWindows/EditorWindowManager.h\"" << NEW_LINE;
            for (const std::string& str : editorWindowClasses)
            {
                file << "#include \"Editor/EditorWindows/" + str + ".h\"" << NEW_LINE;
            }
            file << NEW_LINE;

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

            return true;
        }
        else
        {
            return false;
        }
    }
};