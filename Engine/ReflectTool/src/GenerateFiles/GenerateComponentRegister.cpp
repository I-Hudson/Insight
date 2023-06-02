#include "GenerateFiles/GenerateComponentRegister.h"

#include "Utils.h"

#include "Editor/HotReload/HotReloadExportFunctions.h"

#include <CodeGenerate/CodeGenerate.h>

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateComponentRegister::Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> componentFiles;
        std::vector<Reflect::ReflectContainerData> componentClasses;
        for (const auto& fileParsed : fileParser.GetAllFileParsedData())
        {
            for (const auto& reflectData : fileParsed.ReflectData)
            {
                if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [](const Reflect::ReflectInheritanceData& data)
                    {
                        return "Component" == data.Name;
                    }) != reflectData.Inheritance.end())
                {
                    componentFiles.push_back(fileParsed.FilePath + "/" + fileParsed.FileName + ".h");
                    componentClasses.push_back(reflectData);
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
            Utils::WriteIncludeFiles(file, fileOutputAbsPath, componentFiles);

            file << NEW_LINE;

            file << "namespace Insight {\n";
            TAB_N(1);
            file << "namespace ECS {\n";

            Utils::WriteSourceFunctionDefinition(file, "void", ComponentRegister::c_RegisterAllComponents, {}, [&](std::fstream& file)
            {
                for (const Reflect::ReflectContainerData& reflectData : componentClasses)
                {
                    TAB_N(3);
                    file << "ComponentRegistry::RegisterComponent(::" + reflectData.NameWithNamespace + "::Type_Name, []() { return ::New<::" + reflectData.NameWithNamespace + ", Insight::Core::MemoryAllocCategory::ECS>(); });" << NEW_LINE;
                }
            }, 2);

            Utils::WriteSourceFunctionDefinition(file, "void", ComponentRegister::c_UnregisterAllComponents, {}, [&](std::fstream& file)
            {
                for (const Reflect::ReflectContainerData& reflectData : componentClasses)
                {
                    TAB_N(3);
                    file << "ComponentRegistry::UnregisterComponent(::" + reflectData.NameWithNamespace + "::Type_Name);" << NEW_LINE;
                }
            }, 2);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", ComponentRegister::c_GetAllComponentNames, {}, [&](std::fstream& file)
                {
                    TAB_N(3);
                    file << "std::vector<std::string> componentNames;" << NEW_LINE;
                    for (const Reflect::ReflectContainerData& reflectData : componentClasses)
                    {
                        TAB_N(3);
                        file << "componentNames.push_back(::" + reflectData.NameWithNamespace + "::Type_Name);" << NEW_LINE;
                    }
                    TAB_N(3);
                    file << "return componentNames;" << NEW_LINE;
                }, 2);

            WriteGetTypeInfos(file, componentClasses);

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

    void GenerateComponentRegister::WriteGetTypeInfos(std::fstream& file, const std::vector<Reflect::ReflectContainerData>& componentClasses) const
    {
        Utils::WriteIncludeLibraryFile(file, "Reflect.h");

        Utils::WriteSourceFunctionDefinition(file, "std::vector<Reflect::ReflectTypeInfo>", ComponentRegister::c_GetAllComponentTypeInfos, {}, [&](std::fstream& file)
            {
                TAB_N(3);
                file << "std::vector<Reflect::ReflectTypeInfo> typeInfos;" << NEW_LINE;
                for (const Reflect::ReflectContainerData& reflectData : componentClasses)
                {
                    TAB_N(3);
                    file << "typeInfos.push_back(::" + reflectData.NameWithNamespace + "::GetStaticTypeInfo());" << NEW_LINE;
                }
                TAB_N(3);
                file << "return typeInfos;" << NEW_LINE;
            }, 2);
    }
}