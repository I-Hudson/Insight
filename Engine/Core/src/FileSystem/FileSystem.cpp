#include "FileSystem/FileSystem.h"

#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include <filesystem>

namespace Insight
{
    namespace FileSystem
    {
        void FileSystem::CreateFolder(const std::string& path)
        {
            if (Exists(path))
            {
                return;
            }
            std::error_code errorCode;
            std::filesystem::create_directory(path, errorCode);
            if (errorCode)
            {
                IS_CORE_ERROR("[FileSystem::CreateFolder] Error code: '{}', Message: '{}'.", errorCode.value(), errorCode.message());
            }
        }

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

        std::string_view FileSystem::GetFileExtension(const std::string& file)
        {
            return GetFileExtension(std::string_view(file));
        }
        std::string_view FileSystem::GetFileExtension(std::string_view file)
        {
            if (file.empty() || !IsFile(file))
            {
                return std::string_view();
            }

            std::filesystem::path path(file);
            if (!path.has_extension())
            {
                return std::string_view();
            }

            u64 lastDot = file.find_last_of('.');
            if (lastDot == std::string::npos)
            {
                return std::string_view();
            }

            std::string_view extension = file.substr(lastDot);
            return extension;
        }

        std::string FileSystem::GetAbsolutePath(const std::string& path)
        {
            return GetAbsolutePath(std::string_view(path));
        }
        std::string FileSystem::GetAbsolutePath(std::string_view path)
        {
            std::filesystem::path fsPath = std::filesystem::absolute(std::filesystem::path(path));
            std::string absPath = fsPath.string();
            PathToUnix(absPath);
            return absPath;
        }

        void FileSystem::PathToUnix(std::string& path)
        {
            std::replace(path.begin(), path.end(), '\\', '/');
            if (path.back() == '/')
            {
                path.pop_back();
            }
        }

        void FileSystem::PathToWindows(std::string& path)
        {
            std::replace(path.begin(), path.end(), '/', '\\');
            if (path.back() == '\\')
            {
                path.pop_back();
            }
        }
    }
}