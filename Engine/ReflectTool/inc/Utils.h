#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <functional>

namespace InsightReflectTool
{
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
    }
}