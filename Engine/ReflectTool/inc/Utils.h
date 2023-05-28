#pragma once

#include <fstream>

namespace InsightReflectTool
{
    namespace Utils
    {
        void WriteGeneratedFileHeader(std::fstream& file);

        void ValidateOutputPath(std::string_view path);
    }
}