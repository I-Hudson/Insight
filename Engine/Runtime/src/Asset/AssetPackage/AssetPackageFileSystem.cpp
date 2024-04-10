#include "Asset/AssetPackage/AssetPackageFileSystem.h"
#include "Asset/AssetInfo.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

namespace Insight
{
    namespace Runtime
    {
        AssetPackageFileSystem::AssetPackageFileSystem(std::string_view packagePath, std::string_view packageName)
            : IAssetPackage(packagePath, packageName, AssetPackageType::FileSystem)
        {
        }

        AssetPackageFileSystem::~AssetPackageFileSystem()
        {
        }

        std::vector<Byte> AssetPackageFileSystem::LoadInteral(const AssetInfo* assetInfo) const
        {
            IS_PROFILE_FUNCTION();

            if (!assetInfo)
            {
                return { };
            }

            // We have no zip handle, we must be indexing loose files on disk.
            return FileSystem::ReadFromFile(assetInfo->GetFullFilePath(), FileType::Binary);;
        }

        // Begin - ISerialisable
        void AssetPackageFileSystem::Serialise(Serialisation::ISerialiser* serialiser)
        {
            IS_LOG_CORE_WARN("[AssetPackageFileSystem::Serialise] Can't serialise a file system asset package.");
        }

        void AssetPackageFileSystem::Deserialise(Serialisation::ISerialiser* serialiser)
        {
            IS_LOG_CORE_WARN("[AssetPackageFileSystem::Serialise] Can't deserialise a file system asset package. All files are already on disk in there raw format.");
        }
        // End - ISerialisable
    }
}