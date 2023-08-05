#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/GUID.h"
#include "Serialisation/Serialiser.h"

#include <string>
#include <unordered_set>

namespace Insight
{
    namespace Runtime
    {
        class AssetPackage;

        /// @brief Asset data which is stored to disk for each asset. This should only contain required meta data
        /// like a unique id for other systems like resources to reference
        class IS_RUNTIME AssetMetaData : public Serialisation::ISerialisable
        {
        public:
            constexpr static const char* c_FileExtension = ".is_meta";
            IS_SERIALISABLE_H(AssetMetaData);

            operator bool() const;
            bool IsValid() const;

            /// @brief Unique guid for the asset
            Core::GUID AssetGuid;
            /// @brief Store a vector of Guids which are dependent on this asset.
            /// Example: A single mesh within a model. If the mesh is needed then the model
            /// must be loaded.
            std::unordered_set<Core::GUID> DependentGuids;
            /// @brief Source file path is there is one
            std::string SourcePath;
        };

        /// @brief Store relevant information about a asset. The asset could be on disk or
        /// within a Package. This should never be serialised to disk
        class IS_RUNTIME AssetInfo : public ReferenceCountObject
        {
        public:
            AssetInfo() = default;
            AssetInfo(std::string_view filePath, std::string_view packagePath, AssetPackage* assetPackage);
            AssetInfo(const AssetInfo& other) = default;
            AssetInfo(AssetInfo&& other) = default;
            ~AssetInfo() = default;

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
            AssetMetaData MetaData;
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
            AssetPackage* AssetPackage = nullptr;

            /// @brief Is the asset in an engine format
            bool IsEngineFormat = false;
        };
    }
 
    OBJECT_SERIALISER(Runtime::AssetMetaData, 1,
        SERIALISE_PROPERTY(Core::GUID, AssetGuid, 1, 0)
        SERIALISE_SET_PROPERTY(Core::GUID, DependentGuids, 1, 0)
        SERIALISE_PROPERTY(std::string, SourcePath, 1, 0)
    )
}