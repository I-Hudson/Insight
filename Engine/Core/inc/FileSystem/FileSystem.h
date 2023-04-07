#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <string>
#include <string_view>
#include <vector>

namespace Insight
{
    namespace FileSystem
    {
        class IS_CORE FileSystem
        {
        public:

            static void CreateFolder(const std::string& path);

            static bool SaveToFile(const std::vector<Byte>& data, std::string_view filePath);
            static bool SaveToFile(const std::vector<Byte>& data, std::string_view filePath, bool overwrite);

            static bool Exists(const std::string& path);
            static bool Exists(std::string_view path);

            static bool IsDirectory(const std::string& path);
            static bool IsDirectory(std::string_view path);

            static bool IsFile(const std::string& path);
            static bool IsFile(std::string_view path);

            static std::string_view GetFileExtension(const std::string& file);
            static std::string_view GetFileExtension(std::string_view file);

            static std::string ReplaceExtension(std::string_view file, std::string_view extension);

            static std::string GetAbsolutePath(const std::string& path);
            static std::string GetAbsolutePath(std::string_view path);

            static void PathToUnix(std::string& path);

            static void PathToWindows(std::string& path);
        };
    }
}