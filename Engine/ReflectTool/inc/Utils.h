#pragma once

#include <Reflect/Reflect.h>

#include <fstream>
#include <vector>
#include <string>
#include <functional>

namespace InsightReflectTool
{
    struct StateInfo
    {
        static StateInfo& Instance()
        {
            static StateInfo Instance;
            return Instance;
        }

        std::string GetTypeString() const { return IsEngine ? "Engine" : "Project"; }

        bool IsEngine = false;
        bool IsProject = false;
    };

    namespace Utils
    {
        using WriteSourceFunctionBodyFunc = std::function<void(std::fstream& file)>;

        void WriteGeneratedFileHeader(std::fstream& file);

        void WriteIncludeFile(std::fstream& file, std::string_view includeFile);
        void WriteIncludeFiles(std::fstream& file, std::string_view fileOutputAbsPath, const std::vector<std::string>& filesToInclude);

        void WriteIncludeLibraryFile(std::fstream& file, std::string_view includeFile);

        void WriteHeaderFunctionDeclaration(std::fstream& file, std::string_view returnValue, std::string_view functionName, std::vector<std::string_view> arguments, int indent = 0);
        void WriteSourceFunctionDefinition(std::fstream& file, std::string_view returnValue, std::string_view functionName, std::vector<std::string_view> arguments, WriteSourceFunctionBodyFunc func, int indent = 0);

        void ValidateOutputPath(std::string_view path);

        std::vector<std::string> GetAllFilesWithType(const std::string_view type, const Reflect::Parser::FileParser& fileParser);
        std::vector<std::string> GetAllFilesWithTypeRecersive(const std::string_view type, const Reflect::Parser::FileParser& fileParser);

        bool CheckFileInheritsType(std::string_view type, const std::vector<Reflect::Parser::ReflectInheritanceData>& inheritanceData);

        std::vector<Reflect::Parser::ReflectContainerData> GetAllDerivedTypesFromBaseType(std::string_view baseType, const Reflect::Parser::FileParser& fileParser);
    
    }
}