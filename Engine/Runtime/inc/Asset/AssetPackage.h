#pragma once

#include "Asset/AssetInfo.h"
#include "Core/IObject.h"

#include "Serialisation/Serialisers/ISerialiser.h"

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

            std::vector<const AssetInfo*> GetAllAssetInfos() const;

            std::vector<Byte> LoadAsset(std::string_view path) const;
            std::vector<Byte> LoadAsset(Core::GUID guid) const;

            // Begin - ISerialisable -
            virtual void Serialise(Insight::Serialisation::ISerialiser* serialiser) override;
            virtual void Deserialise(Insight::Serialisation::ISerialiser* serialiser) override;
            // End - ISerialisable -

        private:
            std::vector<Byte> LoadInteral(AssetInfo* assetInfo) const;

            void LoadMetaData(AssetInfo* assetInfo);
            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

        private:
            std::string m_packagePath;
            std::string m_packageName;

            std::unordered_map<std::string, AssetInfo*> m_pathToAssetInfo;
            std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfo;

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

                }
                else
                {
                    zip_t* zip = zip_stream_open(nullptr, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
                    for (auto& [path, info] : assetPackage->m_pathToAssetInfo)
                    {
                        ASSERT(zip_entry_open(zip, path.c_str()) == 0);
                        ASSERT(zip_entry_fwrite(zip, path.c_str()) == 0);
                        ASSERT(zip_entry_close(zip) == 0);
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
        SERIALISE_BASE(IObject, 1, 0)
        SERIALISE_COMPLEX_THIS(Serialisation::AssetPackageSerialiser, 1, 0)
    )

}