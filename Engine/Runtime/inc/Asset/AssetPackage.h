#pragma once

#include "Asset/AssetInfo.h"
#include "Core/IObject.h"

#include "Serialisation/Serialisers/ISerialiser.h"

#include "Resource/IResourceManager.h"

#include "FileSystem/FileSystem.h"
#include "Core/Logger.h"

#include <zip.h>

namespace Insight
{
    namespace Runtime
    {
        /// @brief Contain information about all assets with this package and how to load them.
        class IS_RUNTIME AssetPackage : public IObject
        {
        public:
            AssetPackage() = delete;
            AssetPackage(std::string_view packagePath, std::string_view packageName);
            AssetPackage(const AssetPackage& other) = delete;
            ~AssetPackage();

            constexpr static const char* c_FileExtension = ".isassetpackage";

            std::string_view GetPath() const;
            std::string_view GetName() const;

            const AssetInfo* AddAsset(std::string_view path);

            void RemoveAsset(std::string_view path);
            void RemoveAsset(const Core::GUID& guid);

            bool HasAsset(std::string_view path) const;
            bool HasAsset(const Core::GUID& guid) const;

            const AssetInfo* GetAsset(std::string_view path) const;
            const AssetInfo* GetAsset(const Core::GUID& guid) const;

            /// @brief Replace an asset within the package with another asset,
            /// @param oldAsset 
            /// @param newAsset 
            void ReplaceAsset(const AssetInfo* oldAsset, const AssetInfo* newAsset) const;

            std::vector<const AssetInfo*> GetAllAssetInfos() const;

            std::vector<Byte> LoadAsset(std::string_view path) const;
            std::vector<Byte> LoadAsset(Core::GUID guid) const;

            void BuildPackage(std::string_view path);

            // Begin - ISerialisable -
            virtual void Serialise(Insight::Serialisation::ISerialiser* serialiser) override;
            virtual void Deserialise(Insight::Serialisation::ISerialiser* serialiser) override;
            // End - ISerialisable -

        private:
            std::vector<Byte> LoadInteral(const AssetInfo* assetInfo) const;

            Core::GUID GetGuidFromPath(std::string_view path) const;

            void LoadMetaData(AssetInfo* assetInfo);
            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

        private:
            std::string m_packagePath;
            std::string m_packageName;

            std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfo;
            std::unordered_map<std::string, Core::GUID> m_pathToAssetGuid;

            zip_t* m_zipHandle = nullptr;

            IS_SERIALISABLE_FRIEND;
        };
    }

    namespace Serialisation
    {
        struct AssetPackageSerialiser { };
        template<>
        struct ComplexSerialiser<AssetPackageSerialiser, void, Runtime::AssetPackage>
        {
            void operator()(ISerialiser* serialiser, Runtime::AssetPackage* assetPackage) const
            {
                if (serialiser->IsReadMode())
                {
                    assetPackage->m_zipHandle = zip_open(assetPackage->m_packagePath.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
                    if (!assetPackage->m_zipHandle)
                    {
                        IS_CORE_ERROR("[AssetPackageSerialiser] Failed to open zip at '{}'.", assetPackage->m_packagePath.c_str());
                        return;
                    }

                    i64 entrySize = zip_entries_total(assetPackage->m_zipHandle);
                    for (i64 i = 0; i < entrySize; ++i)
                    {
                        ASSERT(zip_entry_openbyindex(assetPackage->m_zipHandle, i) == 0);
                        const char* path = zip_entry_name(assetPackage->m_zipHandle);

                        std::string assetPath = FileSystem::GetParentPath(assetPackage->m_packagePath) + "/" + path;
                        if (!FileSystem::GetExtension(assetPath).empty())
                        {
                            assetPackage->AddAsset(assetPath);
                        }

                        ASSERT(zip_entry_close(assetPackage->m_zipHandle) == 0);
                    }
                }
                else
                {
                    zip_t* zip = zip_stream_open(nullptr, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
                    for (auto& [guid, info] : assetPackage->m_guidToAssetInfo)
                    {
                        std::string entryPath = FileSystem::GetRelativePath(info->GetFullFilePath(), FileSystem::GetParentPath(assetPackage->m_packagePath));
                        ASSERT(zip_entry_open(zip, entryPath.c_str()) == 0);
                        
                        // Write data file to zip.
                        ASSERT(zip_entry_fwrite(zip, info->GetFullFilePath().c_str()) == 0);
                        ASSERT(zip_entry_close(zip) == 0);

                        // Write meta file to zip.
                        std::string metaFile = FileSystem::ReplaceExtension(info->GetFullFilePath(), Runtime::IResourceManager::c_FileExtension);
                        //zip_entry_fwrite(zip, metaFile.c_str());

                    }

                    char* outbuf = NULL;
                    size_t outbufsize = 0;
                    /* copy compressed stream into outbuf */
                    zip_stream_copy(zip, (void**)&outbuf, &outbufsize);

                    zip_stream_close(zip);

                    std::vector<Byte> data;
                    data.resize(outbufsize);
                    Platform::MemCopy(data.data(), outbuf, outbufsize);

                    free(outbuf);

                    serialiser->Write("", data, false);
                }
            }
        };
    }

    OBJECT_SERIALISER(Runtime::AssetPackage, 1,
        SERIALISE_COMPLEX_THIS(Serialisation::AssetPackageSerialiser, 1, 0)
    )

}