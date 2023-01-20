#include "FileSystem/FileSystem.h"

#include <filesystem>

namespace Insight
{
    namespace FileSystem
    {
        bool FileSystem::Exists(const std::string& path)
        {
            return Exists(std::string_view(path));
        }
        bool FileSystem::Exists(std::string_view path)
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
            return IsDirectory(std::string_view(path));
        }
        bool FileSystem::IsDirectory(std::string_view path)
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
            return IsFile(std::string_view(path));
        }
        bool FileSystem::IsFile(std::string_view path)
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
            return GetAbsolutePath(std::string_view(path));
        }
        std::string FileSystem::GetAbsolutePath(std::string_view path)
        {
            std::filesystem::path fsPath = std::filesystem::absolute(std::filesystem::path(path));
            std::string absPath = fsPath.string();
            return PathToUnix(absPath);
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

        std::string FileSystem::PathToWindow(std::string& path)
        {
            std::replace(path.begin(), path.end(), '/', '\\');
            if (path.back() == '\\')
            {
                path.pop_back();
            }
            return path;
        }
    }
}