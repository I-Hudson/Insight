#include "GenerateFiles/GenerateComponentRegister.h"

#include "Utils.h"

#include <CodeGenerate/CodeGenerate.h>

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateComponentRegister::Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> componentClasses;
        for (const auto& fileParsed : fileParser.GetAllFileParsedData())
        {
            for (const auto& reflectData : fileParsed.ReflectData)
            {
                if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [](const Reflect::ReflectInheritanceData& data)
                    {
                        return "Component" == data.Name;
                    }) != reflectData.Inheritance.end())
                {
                    componentClasses.push_back(reflectData.Name);
                }
            }
        }

        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

            file << "#include \"ECS/Entity.h\"" << NEW_LINE;
            for (const std::string& str : componentClasses)
            {
                file << "#include \"ECS/Components/" + str + ".h\"" << NEW_LINE;
            }
            file << NEW_LINE;

            file << "namespace Insight {\n";
            TAB_N(1);
            file << "namespace ECS {\n";

            TAB_N(2);
            file << "void RegisterAllComponents()" << NEW_LINE;
            TAB_N(2);
            file << "{" << NEW_LINE;

            for (const std::string& str : componentClasses)
            {
                TAB_N(3);
                file << "ComponentRegistry::RegisterComponent(" + str + "::Type_Name, []() { return ::New<" + str + ", Insight::Core::MemoryAllocCategory::ECS>(); });" << NEW_LINE;
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