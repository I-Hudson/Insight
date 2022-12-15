#include "FileSystem/FileSystem.h"

#include <filesystem>

namespace Insight
{
    namespace FileSystem
    {
        std::string FileSystem::GetAbsolutePath(const std::string& path)
        {
            return PathToUnix(std::filesystem::absolute(path).u8string());
        }

        std::string FileSystem::PathToUnix(std::string& path)
        {
            std::replace(path.begin(), path.end(), '\\', '/');
            if (path.back() == '/')
            {
                path.pop_back();
            }
            return path;
        }
    }
}