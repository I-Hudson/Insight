#include "GenerateFiles/GenerateTypeDrawerReigster.h"
#include "Editor/HotReload/HotReloadExportFunctions.h"
#include "Utils.h"

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateTypeDrawerReigster::Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> typeDrawerFiles;
        std::vector<Reflect::Parser::ReflectContainerData> typeDrawerClasses;
        for (const auto& fileParsed : fileParser.GetAllFileParsedData())
        {
            if (fileParsed.parserOptions.DoNotReflect)
            {
                continue;
            }

            for (const auto& reflectData : fileParsed.ReflectData)
            {
                if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [](const Reflect::Parser::ReflectInheritanceData& data)
                    {
                        return "ITypeDrawer" == data.Name;
                    }) != reflectData.Inheritance.end())
                {
                    typeDrawerFiles.push_back(fileParsed.FilePath + "/" + fileParsed.FileName + ".h");
                    typeDrawerClasses.push_back(reflectData);
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

            file << "#include \"ECS/Entity.h\"" << NEW_LINE;
            Utils::WriteIncludeFiles(file, fileOutputAbsPath, typeDrawerFiles);

            file << NEW_LINE;

            file << "namespace Insight {\n";
            TAB_N(1);
            file << "namespace Editor {\n";

            Utils::WriteSourceFunctionDefinition(file, "void", TypeDrawerRegister::c_RegisterAllTypeDrawers, {}, [&](std::fstream& file)
            {
                        for (const Reflect::Parser::ReflectContainerData& reflectData : typeDrawerClasses)
                        {
                            TAB_N(3);
                            file << "TypeDrawerRegister::Instance().RegisterTypeDrawer<::" + reflectData.NameWithNamespace + ">();" << NEW_LINE;
                        }
            }, 2);

            Utils::WriteSourceFunctionDefinition(file, "void", TypeDrawerRegister::c_UnregisterAllTypeDrawers, {}, [&](std::fstream& file)
            {
                        for (const Reflect::Parser::ReflectContainerData& reflectData : typeDrawerClasses)
                        {
                            TAB_N(3);
                            file << "TypeDrawerRegister::Instance().UnregisterTypeDrawer<::" + reflectData.NameWithNamespace + ">();" << NEW_LINE;
                        }
            }, 2);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", TypeDrawerRegister::c_GetAllTypeDrawerNames, {}, [&](std::fstream& file)
                {
                            TAB_N(3);
                            file << "std::vector<std::string> typeDrawerNames;" << NEW_LINE;
                            for (const Reflect::Parser::ReflectContainerData& reflectData : typeDrawerClasses)
                            {
                                TAB_N(3);
                                file << "typeDrawerNames.push_back(::" + reflectData.NameWithNamespace + "::GetTypeName());" << NEW_LINE;
                            }
                            TAB_N(3);
                            file << "return typeDrawerNames;" << NEW_LINE;
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
