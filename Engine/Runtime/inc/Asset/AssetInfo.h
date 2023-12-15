#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/GUID.h"
#include "Serialisation/Serialiser.h"

#include "Reflect.h"

#include <string>
#include <unordered_set>

namespace Insight
{
    namespace Runtime
    {
        class IAssetPackage;

        /// @brief Asset data which is stored to disk for each asset. This should only contain required meta data
        /// like a unique id for other systems like resources to reference
        class IS_RUNTIME AssetMetaData : public Serialisation::ISerialisable
        {
        public:
            AssetMetaData() = default;
            AssetMetaData(const AssetMetaData& other) = default;
            AssetMetaData(AssetMetaData&& other) = default;
            ~AssetMetaData() = default;

            constexpr static const char* c_FileExtension = ".assetmeta";
            IS_SERIALISABLE_H(AssetMetaData);

            operator bool() const;
            bool IsValid() const;

            Core::GUID AssetGuid;
        };

        /// @brief Store relevant information about a asset. The asset could be on disk or
        /// within a Package. This should never be serialised to disk.
        /// AssetInfo's are created for disk resource which can either be loader as a resource via a IResourceLoader
        /// or if an object has specified an extension to be a valid Asset.
        /// Example is *.isworld should not be a resource as it shouldn't be loaded via ResourceManager but is loaded via 
        /// WorldSystem. As it is not loaded from a IResourceLoader any file with the extension *.isworld is not tracked as a AssetInfo
        /// meaning we need to manually tell the AssetRegistry to track *.isworld files. 
        /// 
        /// NOTE: Maybe all files should be tracked via AssetRegistry? Do we really want to know about files which can't be natively loaded
        /// by the engine? Or if an asset doesn't have a loader which can load it, should the asset be loadable by a default loader and result in a 
        /// default asset which just stores the file data? (This one seems like the best for now)
        class IS_RUNTIME AssetInfo : public ReferenceCountObject
        {
        public:
            AssetInfo() = delete;
            AssetInfo(std::string_view filePath, std::string_view packagePath, IAssetPackage* assetPackage);
            AssetInfo(const AssetInfo& other) = default;
            AssetInfo(AssetInfo&& other) = default;
            ~AssetInfo();

            operator bool() const;
            bool IsValid() const;

            bool operator==(const AssetInfo& other)const;
            bool operator!=(const AssetInfo& other)const;

            void SetFile(std::string fileName, std::string filePath);
            void SetPackage(std::string fullPackagePath);

            /// @brief Return the full path to the asset, relative from either the content folder
            /// or package if it is stored in one
            /// @return std::string
            std::string GetFullFilePath() const;
            /// @brief Return the full path to the package, relative from the content folder
            /// @return std::string
            std::string GetFullPackagePath() const;

            /// @brief MetaData stored on disk for this resource.
            AssetMetaData* MetaData = nullptr;
            /// @brief Should the asset care about meta data. If false no meta data file with be created.
            /// False should be used for engine/editor assets.
            bool EnableMetaData;

            /// @brief Filename of the asset with extension but no path
            std::string FileName;
            /// @brief Path to the asset without the filename or extension
            std::string FilePath;

            /// @brief Package name with extension, but no path
            std::string PackageName;
            /// @brief Path to the package without the package name or extension
            std::string PackagePath;
            /// @brief Pointer to the asset package this asset is in.
            IAssetPackage* AssetPackage = nullptr;

            /// @brief Is the asset in an engine format
            bool IsEngineFormat = false;

            Core::GUID Guid;

        private:
            void LoadMetaData() const;
        };
    }
 
    OBJECT_SERIALISER(Runtime::AssetMetaData, 1,
        SERIALISE_PROPERTY(Core::GUID, AssetGuid, 1, 0)
    )
}