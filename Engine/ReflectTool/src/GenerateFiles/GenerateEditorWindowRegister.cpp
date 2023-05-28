#include "GenerateFiles/GenerateEditorWindowRegister.h"

#include "Utils.h"

#include <CodeGenerate/CodeGenerate.h>

#include <string>
#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateEditorWindowRegister::Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> editorWindowClasses;
        for (const auto& fileParsed : fileParser.GetAllFileParsedData())
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
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

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
}