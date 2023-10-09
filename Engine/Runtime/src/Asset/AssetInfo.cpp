#include "Asset/AssetInfo.h"

#include "FileSystem/FileSystem.h"

namespace Insight
{
    namespace Runtime
    {
        AssetInfo::AssetInfo(std::string_view filePath, std::string_view packagePath, Insight::Runtime::AssetPackage* assetPackage)
        {
            std::string absFilePath = std::string(filePath);
            FileSystem::PathToUnix(absFilePath);

            std::string absPackagePath = std::string(packagePath);
            FileSystem::PathToUnix(absPackagePath);

            FileName = FileSystem::GetFileName(absFilePath);
            FilePath = FileSystem::GetParentPath(absFilePath);

            PackageName = FileSystem::GetFileName(absPackagePath);
            PackagePath = FileSystem::GetParentPath(absPackagePath);

            ASSERT(assetPackage);
            AssetPackage = assetPackage;
        }

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

        void AssetInfo::SetFile(std::string fileName, std::string filePath)
        {
            FileName = std::move(fileName);
            FilePath = std::move(filePath);
        }

        void AssetInfo::SetPackage(std::string fullPackagePath)
        {
            PackageName = FileSystem::GetFileName(fullPackagePath);
            PackagePath = FileSystem::GetParentPath(fullPackagePath);
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
}