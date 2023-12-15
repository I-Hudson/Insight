#include "Asset/AssetPackage/AssetPackageZip.h"
#include "Asset/AssetInfo.h"

#include "FileSystem/FileSystem.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"

namespace Insight
{
    namespace Runtime
    {
        AssetPackageZip::AssetPackageZip(std::string_view packagePath, std::string_view packageName)
            : IAssetPackage(packagePath, packageName, AssetPackageType::Zip)
        {
        }

        AssetPackageZip::~AssetPackageZip()
        {
            if (m_zipHandle)
            {
                zip_close(m_zipHandle);
                m_zipHandle = nullptr;
            }
        }

        void AssetPackageZip::BuildPackage(std::string_view path)
        {
            m_packagePath = path;
            Serialisation::BinarySerialiser serialiser(false);
            serialiser.SetObjectTracking(false);

            ::Insight::Serialisation::SerialiserObject<AssetPackageZip> serialiserObject;
            serialiserObject.MetaDataEnabled = false;
            serialiserObject.Serialise(&serialiser, *this);

            FileSystem::SaveToFile(serialiser.GetRawData(), path, FileType::Binary, true);
        }

        std::vector<Byte> AssetPackageZip::LoadInteral(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                return { };
            }

            if (m_zipHandle)
            {
                std::string packageParent = FileSystem::GetParentPath(m_packagePath);
                std::string relativePath = FileSystem::GetRelativePath(assetInfo->GetFullFilePath(), packageParent);

                ASSERT(zip_entry_open(m_zipHandle, relativePath.c_str()) == 0);
                u64 dataUncompSize = zip_entry_uncomp_size(m_zipHandle);

                std::vector<Byte> data;
                data.resize(dataUncompSize);

                i64 readSize = zip_entry_noallocread(m_zipHandle, data.data(), data.size());
                ASSERT(readSize == dataUncompSize);

                ASSERT(zip_entry_close(m_zipHandle) == 0);

                return data;
            }
        }

        IS_SERIALISABLE_CPP(AssetPackageZip)
    }
}