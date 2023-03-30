#pragma once

#include "Core/Defines.h"

#include <string>
#include <string_view>

namespace Insight
{
    namespace FileSystem
    {
        class IS_CORE FileSystem
        {
        public:

            static void CreateFolder(const std::string& path);

            static bool Exists(const std::string& path);
            static bool Exists(std::string_view path);

            static bool IsDirectory(const std::string& path);
            static bool IsDirectory(std::string_view path);

            static bool IsFile(const std::string& path);
            static bool IsFile(std::string_view path);

            static std::string GetAbsolutePath(const std::string& path);
            static std::string GetAbsolutePath(std::string_view path);

            static void PathToUnix(std::string& path);

            static void PathToWindows(std::string& path);
        };
    }
}