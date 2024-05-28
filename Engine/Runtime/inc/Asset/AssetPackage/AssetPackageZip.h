#pragma once

#include "Asset/AssetPackage/IAssetPackage.h"
#include "Asset/AssetRegistry.h"

#include "Platforms/Platform.h"

#include "FileSystem/FileSystem.h"
#include <zip.h>

namespace Insight
{
    namespace Runtime
    {
        /// @brief Contain information about all assets with this package and how to load them.
        class IS_RUNTIME AssetPackageZip : public IAssetPackage
        {
        public:
            AssetPackageZip() = delete;
            AssetPackageZip(std::string_view packagePath, std::string_view packageName);
            AssetPackageZip(const AssetPackageZip& other) = delete;
            virtual ~AssetPackageZip() override;

            IS_SERIALISABLE_H(AssetPackageZip);

            virtual void BuildPackage(std::string_view path) override;

        private:
            virtual std::vector<Byte> LoadInteral(const AssetInfo* assetInfo) const override;

            zip_t* m_zipHandle = nullptr;

            friend IAssetPackage;
        };
    }

    namespace Serialisation
    {
        struct AssetPackageSerialiser { };
        template<>
        struct ComplexSerialiser<AssetPackageSerialiser, void, Runtime::AssetPackageZip>
        {
            void operator()(ISerialiser* serialiser, Runtime::AssetPackageZip* assetPackage) const
            {
                if (serialiser->IsReadMode())
                {
                    assetPackage->m_zipHandle = zip_open(assetPackage->m_packagePath.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
                    if (!assetPackage->m_zipHandle)
                    {
                        //IS_LOG_CORE_ERROR("[AssetPackageSerialiser] Failed to open zip at '{}'.", assetPackage->m_packagePath.c_str());
                        return;
                    }

                    std::vector<std::string> assetPaths;
                    i64 entrySize = zip_entries_total(assetPackage->m_zipHandle);
                    for (i64 i = 0; i < entrySize; ++i)
                    {
                        ASSERT(zip_entry_openbyindex(assetPackage->m_zipHandle, i) == 0);
                        const char* path = zip_entry_name(assetPackage->m_zipHandle);

                        std::string assetPath = FileSystem::GetParentPath(assetPackage->m_packagePath) + "/" + path;
                        if (!FileSystem::GetExtension(assetPath).empty())
                        {
                            assetPaths.push_back(assetPath);
                        }

                        ASSERT(zip_entry_close(assetPackage->m_zipHandle) == 0);
                    }

                    std::sort(assetPaths.begin(), assetPaths.end(), [](const std::string& strA, const std::string& strB)
                        {
                                return FileSystem::GetExtension(strA) == Runtime::AssetMetaData::c_FileExtension
                                    && FileSystem::GetExtension(strB) != Runtime::AssetMetaData::c_FileExtension;
                        });
                    for (const std::string& path : assetPaths)
                    {
                        Runtime::AssetRegistry::Instance().AddAssetFromPackage(path, assetPackage);
                    }
                }
                else
                {
                    zip_t* zip = zip_stream_open(nullptr, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
                    for (Runtime::AssetInfo* assetInfo : assetPackage->m_assetInfos)
                    {
                        std::string entryPath = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), FileSystem::GetParentPath(assetPackage->m_packagePath));
                        int errorCode = zip_entry_open(zip, entryPath.c_str());
                        if (errorCode != 0)
                        {
                            continue;
                        }

                        // Write data file to zip.
                        errorCode = zip_entry_fwrite(zip, assetInfo->GetFullFilePath().c_str());
                        if (errorCode != 0)
                        {
                            errorCode = zip_entry_close(zip);
                        }
                        else
                        {
                            errorCode = zip_entry_close(zip);
                        }
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

    OBJECT_SERIALISER(Runtime::AssetPackageZip, 1,
        SERIALISE_COMPLEX_THIS(Serialisation::AssetPackageSerialiser, 1, 0)
    )
}