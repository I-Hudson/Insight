#include "Asset/AssetInfo.h"

#include "FileSystem/FileSystem.h"

namespace Insight::Runtime
{
    IS_SERIALISABLE_CPP(AssetMetaData);

    AssetMetaData::operator bool() const
    {
        return IsValid();
    }

    bool AssetMetaData::IsValid() const
    {
        return AssetGuid.IsValid();
    }

    AssetInfo::operator bool() const
    {
        return IsValid();
    }

    bool AssetInfo::IsValid() const
    {
        return !FileName.empty()
            && !FilePath.empty();
    }

    bool AssetInfo::operator==(const AssetInfo& other) const
    {
        return FileName == other.FileName
            && FilePath == other.FilePath
            && PackageName == other.PackageName
            && PackagePath == other.PackagePath;
    }

    bool AssetInfo::operator!=(const AssetInfo& other) const
    {
        return !(*this == other);
    }

    void AssetInfo::SetFile(std::string fullFilePath)
    {
        FileName = FileSystem::GetFileName(fullFilePath);
        FilePath = FileSystem::GetParentPath(fullFilePath);
    }

    void AssetInfo::SetPackage(std::string fullPackagePath)
    {
        PackageName = FileSystem::GetFileName(fullPackagePath);
        PackagePath = FileSystem::GetParentPath(fullPackagePath);
    }

    void AssetInfo::SetSource(std::string fullSourcePath)
    {
        SourceAssetName = FileSystem::GetFileName(fullSourcePath);
        SourceAssetPath = FileSystem::GetParentPath(fullSourcePath);
    }

    std::string AssetInfo::GetFullFilePath() const
    {
        return FilePath + "/" + FileName;
    }

    std::string AssetInfo::GetFullPackagePath() const
    {
        return PackagePath + "/" + PackageName;
    }
}