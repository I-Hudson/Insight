#include "GenerateFiles/GenerateResourceRegister.h"
#include "Editor/HotReload/HotReloadExportFunctions.h"
#include "Utils.h"

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateResourceRegister::Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::vector<std::string> resourceFiles = Utils::GetAllFilesWithType("IResource", fileParser);
        std::vector<Reflect::Parser::ReflectContainerData> resourceClasses = Utils::GetAllDerivedTypesFromBaseType("IResource", fileParser);

        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);
            std::string fileOutputAbsPath = std::filesystem::canonical(outFilePath).parent_path().generic_string();

            file << "#include \"Resource/Resource.h\"" << NEW_LINE;
            file << "#include \"Resource/ResourceTypeId.h\"" << NEW_LINE;
            Utils::WriteIncludeFiles(file, fileOutputAbsPath, resourceFiles);

            file << NEW_LINE;

            file << "namespace Insight {\n";
            TAB_N(1);
            file << "namespace Runtime {\n";

            Utils::WriteSourceFunctionDefinition(file, "void", ResourceRegister::c_RegisterAllResources, {}, [&](std::fstream& file)
            {
                        for (const Reflect::Parser::ReflectContainerData& reflectData : resourceClasses)
                        {
                            TAB_N(3);
                            file << "ResourceRegister::RegisterResource<::" + reflectData.NameWithNamespace + ">();" << NEW_LINE;
                        }
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
