#pragma once

#include "Runtime/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/GUID.h"
#include "Serialisation/Serialiser.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        /// @brief Asset data which is stored to disk for each asset. This should only contain required meta data
        /// like a unique id for other systems like resources to reference
        class IS_RUNTIME AssetMetaData : public Serialisation::ISerialisable
        {
        public:
            constexpr static const char* c_FileExtension = ".meta";
            IS_SERIALISABLE_H(AssetMetaData);

            operator bool() const;
            bool IsValid() const;

            /// @brief Unique guid for the asset
            Core::GUID AssetGuid;
            /// @brief Store a vector of Guids which are dependent on this asset.
            /// Example: A single mesh within a model. If the mesh is needed then the model
            /// must be loaded.
            std::vector<Core::GUID> DependentGuids;
            /// @brief Source file path is there is one
            std::string SourcePath;
        };

        /// @brief Store relevant information about a asset. The asset could be on disk or
        /// within a Package. This should never be serialised to disk
        class IS_RUNTIME AssetInfo
        {
        public:
            AssetInfo() = default;
            AssetInfo(const AssetInfo& other) = default;
            AssetInfo(AssetInfo&& other) = default;
            ~AssetInfo() = default;

            operator bool() const;
            bool IsValid() const;

            bool operator==(const AssetInfo& other)const;
            bool operator!=(const AssetInfo& other)const;

            void SetFile(std::string fullFilePath);
            void SetPackage(std::string fullPackagePath);
            void SetSource(std::string fullSourcePath);

            /// @brief Return the full path to the asset, relative from either the content folder
            /// or package if it is stored in one
            /// @return std::string
            std::string GetFullFilePath() const;
            /// @brief Return the full path to the package, relative from the content folder
            /// @return std::string
            std::string GetFullPackagePath() const;

            AssetMetaData MetaData;

            /// @brief Filename of the asset with extension but no path
            std::string FileName;
            /// @brief Path to the asset without the filename or extension
            std::string FilePath;

            /// @brief Package name with extension, but no path
            std::string PackageName;
            /// @brief Path to the package without the package name or extension
            std::string PackagePath;

            /// @brief Filename of the source asset is there is one
            std::string SourceAssetName;
            /// @brief Path to the source asset is there is one
            std::string SourceAssetPath;

            /// @brief Is the asset in an engine format
            bool IsEngineFormat = false;
        };
    }
 
    OBJECT_SERIALISER(Runtime::AssetMetaData, 1,
        SERIALISE_PROPERTY(Core::GUID, AssetGuid, 1, 0)
        SERIALISE_ARRAY_PROPERTY(Core::GUID, DependentGuids, 1, 0)
        SERIALISE_PROPERTY(std::string, SourcePath, 1, 0)
    )
}