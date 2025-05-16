#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <string>
#include <string_view>
#include <vector>
#include <fstream>

namespace Insight
{
    enum class FileType
    {
        Binary,
        Text
    };
    constexpr const int c_FileTypeToStdIos[] =
    {
        std::ios::binary,
        0
    };

    class IS_CORE FileSystem
    {
    public:

        static void CreateFolder(const std::string& path);

        static bool SaveToFile(const Byte* data, u64 dataSize, std::string_view filePath, FileType fileType, bool overwrite = false);
        static bool SaveToFile(const std::vector<Byte>& data, std::string_view filePath);
        static bool SaveToFile(const std::vector<Byte>& data, std::string_view filePath, bool overwrite);
        static bool SaveToFile(const std::vector<Byte>& data, std::string_view filePath, FileType fileType, bool overwrite);

        static std::vector<Byte> ReadFromFile(std::string_view filePath);
        static std::vector<Byte> ReadFromFile(std::string_view filePath, FileType fileType);

        static bool Exists(const std::string& path);
        static bool Exists(std::string_view path);

        static bool IsDirectory(const std::string& path);
        static bool IsDirectory(std::string_view path);

        static bool IsFile(const std::string& path);
        static bool IsFile(std::string_view path);

        static u64 GetFileSize(std::string_view path);

        static std::string GetFileName(std::string_view filePath, bool removeExtension = false);
        static std::string GetParentPath(std::string_view path);

        static std::string_view GetFileExtension(const std::string& file);
        static std::string_view GetFileExtension(std::string_view file);
        static std::string_view GetExtension(std::string_view file);

        static std::string ReplaceExtension(std::string_view file, std::string_view extension);

        static bool IsAbsolutePath(std::string_view path);
        static std::string GetAbsolutePath(const std::string& path);
        static std::string GetAbsolutePath(std::string_view path);

        static std::string GetRelativePath(std::string_view path, std::string_view basePath);

        static bool PathIsSubPathOf(std::string_view path, std::string_view basePath);

        static void PathToUnix(std::string& path);
        static void PathToUnix(std::wstring& path);
        static void PathToWindows(std::string& path);
        static void PathToWindows(std::wstring& path);
    };
}