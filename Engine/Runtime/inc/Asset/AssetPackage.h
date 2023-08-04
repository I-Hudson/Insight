#pragma once

#include "Asset/AssetInfo.h"
#include "Core/IObject.h"

#include "Serialisation/Serialisers/ISerialiser.h"

namespace Insight
{
    namespace Runtime
    {
        /// @brief Contain information about all assets with this package and how to load them.
        class AssetPackage : public IObject
        {
        public:
            AssetPackage();
            AssetPackage(std::string_view packagePath, std::string_view packageName);
            AssetPackage(const AssetPackage& other) = delete;
            ~AssetPackage();

            constexpr static const char* c_FileExtension = ".isassetpackage";

            const AssetInfo* AddAsset(std::string_view path);

            void RemoveAsset(std::string_view path);
            void RemoveAsset(const Core::GUID& guid);

            bool HasAsset(std::string_view path) const;
            bool HasAsset(const Core::GUID& guid) const;

            const AssetInfo* GetAsset(std::string_view path) const;
            const AssetInfo* GetAsset(const Core::GUID& guid) const;

            std::vector<Byte> LoadAsset(std::string_view path) const;
            std::vector<Byte> LoadAsset(Core::GUID guid) const;

        private:
            std::vector<Byte> LoadInteral(AssetInfo* assetInfo) const;

            void LoadMetaData(AssetInfo* assetInfo);
            bool AssetInfoValidate(const AssetInfo* assetInfo) const;

            // Begin - ISerialisable -
            virtual void Serialise(Insight::Serialisation::ISerialiser* serialiser) override;
            virtual void Deserialise(Insight::Serialisation::ISerialiser* serialiser) override;
            // End - ISerialisable -

        private:
            std::string m_packagePath;
            std::string m_packageName;

            std::unordered_map<std::string, AssetInfo*> m_pathToAssetInfo;
            std::unordered_map<Core::GUID, AssetInfo*> m_guidToAssetInfo;
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

            }
        };
    }

    OBJECT_SERIALISER(Runtime::AssetPackage, 1,
        SERIALISE_BASE(IObject, 1, 0)
        SERIALISE_COMPLEX_THIS(Serialisation::AssetPackageSerialiser, 1, 0)
    )

}