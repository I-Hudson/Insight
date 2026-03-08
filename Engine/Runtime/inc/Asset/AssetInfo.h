#pragma once

#include "Runtime/Defines.h"

#include "Core/GUID.h"
#include "Serialisation/Serialiser.h"
#include "Serialisation/ReflectSerialisation.h"

#include <Reflect/Reflect.h>

#include <string>

namespace Insight
{
#define IS_ASSET_SUBMETA_DATA_TYPE(TypeName)\
    static const char* GetStaticTypeName() { return STRINGIZE(TypeName); }\
    virtual const char* GetTypeName() override { return STRINGIZE(TypeName); }

    namespace Runtime
    {
        class IAssetPackage;

        /// @brief 
        class IS_RUNTIME IAssetSubMetaData : public Serialisation::ISerialisable
        {
        public:
            IAssetSubMetaData() = default;
            virtual ~IAssetSubMetaData() { }

            IS_SERIALISABLE_H(IAssetSubMetaData);
            virtual const char* GetTypeName() = 0;
        };
 
        /*
        * @brief Asset data which is stored to disk for each asset. This should only contain required meta data
            like a unique id for other systems like resources to reference.

            This stores a list of all meta data stored within the .meta file in 'SubMetaData'. In editor use this can contain anything
            the editor needs while for standalone this will only contain runtime information.
        */
        class IS_RUNTIME AssetMetaData : public Serialisation::ISerialisable
        {
        public:
            AssetMetaData() = default;
            AssetMetaData(const AssetMetaData& other) = default;
            AssetMetaData(AssetMetaData&& other) = default;
            virtual ~AssetMetaData() = default;

            constexpr static const char* c_FileExtension = ".assetmeta";
            IS_SERIALISABLE_H(AssetMetaData);

            operator bool() const;
            bool IsValid() const;

            IAssetSubMetaData* GetSubMetaData(const char* typeName) const;
            template<typename T, std::enable_if_t<std::is_base_of_v<IAssetSubMetaData, T>, int> = 0>
            T* GetSubMetaData() const
            {
                return static_cast<T*>(GetSubMetaData(T::GetStaticTypeName()));
            }

            // Store the core asset meta information here.

            Reflect::Type ReflectType;
            Core::GUID AssetGuid;

            /// @brief Store addtional information other sub systems might want to add.
            std::vector<IAssetSubMetaData*> SubMetaData;
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
            AssetInfo(std::string_view filePath, std::string_view packagePath, IAssetPackage* assetPackage, const bool enableMetaFiles);
            AssetInfo(const AssetInfo& other) = default;
            AssetInfo(AssetInfo&& other) = default;
            ~AssetInfo();

            operator bool() const;
            bool IsValid() const;

            bool operator==(const AssetInfo& other)const;
            bool operator!=(const AssetInfo& other)const;

            void SetFile(std::string fileName, std::string filePath);
            void SetPackage(std::string fullPackagePath);

            bool IsMemoryAsset() const;

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

            std::string FullFilePath;

            /// @brief Package name with extension, but no path
            std::string PackageName;
            /// @brief Path to the package without the package name or extension
            std::string PackagePath;
            /// @brief Pointer to the asset package this asset is in.
            IAssetPackage* AssetPackage = nullptr;

            /// @brief Is the asset in an engine format
            bool IsEngineFormat = false;

            Core::GUID Guid;

            void SaveMetaData() const;
        private:
            void LoadMetaData() const;

            bool m_isMemoryAsset = false;
        };
    }
 
    OBJECT_SERIALISER(Runtime::IAssetSubMetaData, 1,
        );

    OBJECT_SERIALISER(Runtime::AssetMetaData, 3,
        SERIALISE_PROPERTY(Core::GUID, AssetGuid, 1, 0)
        SERIALISE_PROPERTY(Reflect::Type, ReflectType, 2, 0)
        SERIALISE_ARRAY_OBJECT(Runtime::IAssetSubMetaData, SubMetaData, 3, 0)
    );
}