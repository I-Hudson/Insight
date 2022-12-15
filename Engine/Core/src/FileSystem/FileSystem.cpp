#include "FileSystem/FileSystem.h"

#include <filesystem>

namespace Insight
{
    namespace FileSystem
    {
        bool FileSystem::Exists(const std::string& path)
        {
            if (path.empty())
            {
                return false;
            }

            std::error_code errorCode = {};
            return std::filesystem::exists(path, errorCode);
        }

        bool FileSystem::IsDirectory(const std::string& path)
        {
            if (path.empty())
            {
                return false;
            }

            std::error_code existsErrorCode = {};
            std::error_code isDirectoryErrorCode = {};
            return std::filesystem::exists(path, existsErrorCode) && std::filesystem::is_directory(path, isDirectoryErrorCode);
        }

        bool FileSystem::IsFile(const std::string& path)
        {
            if (path.empty())
            {
                return false;
            }

            std::error_code existsErrorCode = {};
            std::error_code isFileErrorCode = {};
            return std::filesystem::exists(path, existsErrorCode) && std::filesystem::is_regular_file(path, isFileErrorCode);
        }

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