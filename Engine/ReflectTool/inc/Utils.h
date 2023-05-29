#pragma once

#include <fstream>
#include <vector>
#include <string>

namespace InsightReflectTool
{
    namespace Utils
    {
        void WriteGeneratedFileHeader(std::fstream& file);
        void WriteIncludeFiles(std::fstream& file, std::string_view fileOutputAbsPath, const std::vector<std::string>& filesToInclude);

        void ValidateOutputPath(std::string_view path);
    }
}