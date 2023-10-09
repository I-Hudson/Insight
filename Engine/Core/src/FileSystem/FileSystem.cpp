#include "FileSystem/FileSystem.h"

#include "Core/TypeAlias.h"
#include "Core/Logger.h"

#include <fstream>
#include <filesystem>

namespace Insight
{
    void FileSystem::CreateFolder(const std::string& path)
    {
        if (Exists(path))
        {
            return;
        }
        std::error_code errorCode;
        std::filesystem::create_directories(GetAbsolutePath(path), errorCode);
        if (errorCode)
        {
            IS_CORE_ERROR("[FileSystem::CreateFolder] Error code: '{}', Message: '{}'.", errorCode.value(), errorCode.message());
        }
    }

    bool FileSystem::SaveToFile(const Byte* data, u64 dataSize, std::string_view filePath, FileType fileType, bool overwrite)
    {
        if (Exists(filePath) && !overwrite)
        {
            return false;
        }

        std::string folderPath = std::filesystem::path(filePath).parent_path().string();
        if (!Exists(folderPath))
        {
            CreateFolder(folderPath);
        }

        std::fstream fileStream;
        fileStream.open(filePath, std::ios::out | c_FileTypeToStdIos[(int)fileType]);
        if (!fileStream.is_open())
        {
            fileStream.close();
            return false;
        }

        fileStream.write((char*)data, dataSize);
        fileStream.close();
        return true;
    }
    bool FileSystem::SaveToFile(const std::vector<Byte>& data, std::string_view filePath)
    {
        return SaveToFile(data, filePath, false);
    }
    bool FileSystem::SaveToFile(const std::vector<Byte>& data, std::string_view filePath, bool overwrite)
    {
        return SaveToFile(data.data(), data.size(), filePath, FileType::Text, overwrite);
    }

    bool FileSystem::SaveToFile(const std::vector<Byte>& data, std::string_view filePath, FileType fileType, bool overwrite)
    {
        return SaveToFile(data.data(), data.size(), filePath, fileType, overwrite);
    }

    std::vector<Byte> FileSystem::ReadFromFile(std::string_view filePath)
    {
        return ReadFromFile(filePath, FileType::Text);
    }

    std::vector<Byte> FileSystem::ReadFromFile(std::string_view filePath, FileType fileType)
    {
        std::vector<Byte> fileData;
        if (!Exists(filePath))
        {
            return fileData;
        }

        std::fstream fileStream;
        fileStream.open(filePath, std::ios::in | c_FileTypeToStdIos[(int)fileType]);
        if (!fileStream.is_open())
        {
            fileStream.close();
            return fileData;
        }

        const u64 fileSize = GetFileSize(filePath);
        fileData.resize(fileSize);

        fileStream.read((char*)fileData.data(), fileSize);

        fileStream.close();
        return fileData;
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
        return std::filesystem::exists(GetAbsolutePath(path), errorCode);
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

    u64 FileSystem::GetFileSize(std::string_view path)
    {
        if (!Exists(path) || !IsFile(path))
        {
            return 0;
        }
        return std::filesystem::file_size(path);
    }

    std::string FileSystem::GetFileName(std::string_view filePath, bool removeExtension)
    {
        std::string fileName = std::string(filePath.substr(filePath.find_last_of('/') + 1));
        if (removeExtension)
        {
            fileName = ReplaceExtension(fileName, "");
        }
        return fileName;
    }

    std::string FileSystem::GetParentPath(std::string_view path)
    {
        u64 index = path.find_last_of('/');
        if (index == std::string::npos)
        {
            index = path.find_last_of('\\');
        }

        if (index == std::string::npos)
        {
            return "";
        }

        return std::string(path.substr(0, index));

        if (!Exists(path))
        {
            u64 lastSlash = path.find_last_of('/');
            if (lastSlash != std::string::npos)
            {
                return std::string(path.substr(0, lastSlash));
            }
            return "";
        }
        return std::filesystem::path(path).parent_path().string();
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

    std::string_view FileSystem::GetExtension(std::string_view file)
    {
        u64 lastDot = file.find_last_of('.');
        u64 lastSlash = file.find_last_of('/');
        if (lastDot == std::string::npos)
        {
            return std::string_view();
        }
        else if (lastSlash != std::string::npos 
            && lastSlash > lastDot)
        {
            return std::string_view();
        }

        std::string_view extension = file.substr(lastDot);
        return extension;
    }

    std::string FileSystem::ReplaceExtension(std::string_view file, std::string_view extension)
    {
        // New Extension must be as least 2 characters. A '.' and another character.
        //if (extension.size() < 2 || extension.at(0) != '.')
        //{
        //    return {};
        //}

        std::string_view fileExtension = GetExtension(file);
        if (fileExtension == extension)
        {
            return std::string(file);
        }
        else if (!fileExtension.empty())
        {
            const u64 lastDot = file.find_last_of('.');
            std::string newPath = std::string(file.substr(0, lastDot));
            newPath += extension;
            return newPath;
        }
        else
        {
            std::string newPath = std::string(file);
            newPath += extension;
            return newPath;
        }
    }

    std::string FileSystem::GetAbsolutePath(const std::string& path)
    {
        return GetAbsolutePath(std::string_view(path));
    }
    std::string FileSystem::GetAbsolutePath(std::string_view path)
    {
        if (path.empty())
        {
            return std::string(path);
        }
        std::filesystem::path fsPath = std::filesystem::absolute(std::filesystem::path(path));
        std::string absPath = fsPath.string();
        PathToUnix(absPath);
        return absPath;
    }

    std::string FileSystem::GetRelativePath(std::string_view path, std::string_view basePath)
    {
        std::error_code errorCode;
        std::filesystem::path fsPath = std::filesystem::relative(path, basePath, errorCode);
        if (errorCode)
        {
            IS_CORE_ERROR("[FileSystem::GetRelativePath] Error code: '{}', Message: '{}'.", errorCode.value(), errorCode.message());
        }
        std::string absPath = fsPath.string();
        PathToUnix(absPath);
        return absPath;
    }

    bool FileSystem::PathIsSubPathOf(std::string_view path, std::string_view basePath)
    {
        std::filesystem::path fsPath(path);
        std::filesystem::path fsBasePath(basePath);

        const auto mismatch_pair = std::mismatch(fsPath.begin(), fsPath.end(), fsBasePath.begin(), fsBasePath.end());
        return mismatch_pair.second == fsBasePath.end();
    }

    void FileSystem::PathToUnix(std::string& path)
    {
        std::replace(path.begin(), path.end(), '\\', '/');
        if (!path.empty() && path.back() == '/')
        {
            path.pop_back();
        }
    }

    void FileSystem::PathToWindows(std::string& path)
    {
        std::replace(path.begin(), path.end(), '/', '\\');
        if (!path.empty() && path.back() == '\\')
        {
            path.pop_back();
        }
    }
}