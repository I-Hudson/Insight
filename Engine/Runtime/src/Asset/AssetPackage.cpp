#include "Asset/AssetPackage.h"
//#include "Asset/AssetRegistry.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"

#include "Algorithm/Vector.h"

namespace Insight
{
    namespace Runtime
    {
        AssetPackage::AssetPackage(std::string_view packagePath, std::string_view packageName)
        {
            m_packagePath = FileSystem::GetAbsolutePath(packagePath);
            m_packageName = packageName;
        }

        AssetPackage::~AssetPackage()
        {
            m_assetInfos.clear();
            if (m_zipHandle != nullptr)
            {
                zip_close(m_zipHandle);
            }
        }

        std::string_view AssetPackage::GetPath() const
        {
            return m_packagePath;
        }

        std::string_view AssetPackage::GetName() const
        {
            return m_packageName;
        }

        const AssetInfo* AssetPackage::AddAsset(std::string_view path)
        {
            if (HasAsset(path))
            {
                IS_CORE_INFO("[AssetPackage::AddAsset] Asset at path '{}' already added to this package.", path.data());
                return GetAsset(path);
            }

            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(path));
            return AddAsset(assetInfo);
        }

        const AssetInfo* AssetPackage::AddAsset(AssetInfo* assetInfo)
        {
            if (assetInfo && !HasAsset(assetInfo))
            {
                assetInfo->AssetPackage->RemoveAsset(assetInfo);
                assetInfo->AssetPackage = this;
                assetInfo->PackageName = m_packageName;
                assetInfo->PackagePath = m_packagePath;

                m_assetInfos.push_back(assetInfo);
            }
            return assetInfo;
        }

        void AssetPackage::RemoveAsset(std::string_view path)
        {
            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(path));
            RemoveAsset(assetInfo);
        }

        void AssetPackage::RemoveAsset(const Core::GUID& guid)
        {
            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(guid));
            RemoveAsset(assetInfo);
        }

        void AssetPackage::RemoveAsset(AssetInfo* assetInfo)
        {
            if (!assetInfo || !HasAsset(assetInfo->Guid))
            {
                return;
            }
            assetInfo->AssetPackage = nullptr;
            assetInfo->PackageName.clear();
            assetInfo->PackagePath.clear();
            ASSERT(Algorithm::VectorRemove(m_assetInfos, assetInfo));
        }

        bool AssetPackage::HasAsset(std::string_view path) const
        {
            return HasAsset(GetGuidFromPath(path));
        }

        bool AssetPackage::HasAsset(const Core::GUID& guid) const
        {
            return Algorithm::VectorContainsIf(m_assetInfos, [&guid](const AssetInfo* assetInfo)
                {
                    if (!assetInfo)
                    {
                        return false;
                    }
                    return guid == assetInfo->Guid;
                });
        }

        bool AssetPackage::HasAsset(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                return nullptr;
            }
            return HasAsset(assetInfo->Guid);
        }

        const AssetInfo* AssetPackage::GetAsset(std::string_view path) const
        {
            std::string formattedPath = std::string(path);
            FileSystem::PathToUnix(formattedPath);

            Core::GUID assetGuid = GetGuidFromPath(formattedPath);
            return GetAsset(assetGuid);
        }

        const AssetInfo* AssetPackage::GetAsset(const Core::GUID& guid) const
        {
            if (auto iter = Algorithm::VectorFindIf(m_assetInfos, [&guid](const AssetInfo* assetInfo)
                {
                    return guid == assetInfo->Guid;
                });
                iter != m_assetInfos.end())
            {
                return *iter;
            }
            return nullptr;
        }

        const AssetInfo* AssetPackage::GetAsset(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                return nullptr;
            }
            return GetAsset(assetInfo->Guid);
        }

        void AssetPackage::ReplaceAsset(AssetInfo* oldAsset, AssetInfo* newAsset)
        {
            auto iter = Algorithm::VectorFind(m_assetInfos, oldAsset);
            if (iter == m_assetInfos.end())
            {
                return;
            }
            RemoveAsset(oldAsset);
            AddAsset(newAsset);
            //AssetInfo*& info = *iter;
            //info = RemoveConst(newAsset);
        }

        const std::vector<AssetInfo*>&AssetPackage::GetAllAssetInfos() const
        {
            return m_assetInfos;
        }

        std::vector<Byte> AssetPackage::LoadAsset(std::string_view path) const
        {
            const AssetInfo* assetInfo = GetAsset(GetGuidFromPath(path));
            if (!assetInfo)
            {
                return {};
            }
            return LoadInteral(assetInfo);
        }

        std::vector<Byte> AssetPackage::LoadAsset(Core::GUID guid) const
        {
            const AssetInfo* assetInfo = GetAsset(guid);
            if (!assetInfo)
            {
                return {};
            }
            return LoadInteral(assetInfo);
        }

        void AssetPackage::BuildPackage(std::string_view path)
        {
            m_packagePath = path;
            Serialisation::BinarySerialiser serialiser(false);
            serialiser.SetObjectTracking(false);

            ::Insight::Serialisation::SerialiserObject<AssetPackage> serialiserObject;
            serialiserObject.MetaDataEnabled = false;
            serialiserObject.Serialise(&serialiser, *this);

            FileSystem::SaveToFile(serialiser.GetRawData(), path, FileType::Binary, true);
        }

        std::vector<Byte> AssetPackage::LoadInteral(const AssetInfo* assetInfo) const
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
            else
            {
                // We have no zip handle, we must be indexing loose files on disk.
                return FileSystem::ReadFromFile(assetInfo->GetFullFilePath(), FileType::Binary);
            }
        }

        Core::GUID AssetPackage::GetGuidFromPath(std::string_view path) const
        {
            if (auto iter = Algorithm::VectorFindIf(m_assetInfos, [&path](const AssetInfo* assetInfo)
                {
                    return path == assetInfo->GetFullFilePath();
                });
                iter != m_assetInfos.end())
            {
                return (*iter)->Guid;
            }
            return { };
        }

        void AssetPackage::LoadMetaData(AssetInfo* assetInfo)
        {
            return;
            if (!AssetInfoValidate(assetInfo))
            {
                return;
            }

            Serialisation::BinarySerialiser binarySerialiser(true);

            std::string metaFilePath = assetInfo->GetFullFilePath() + AssetMetaData::c_FileExtension;
            if (!FileSystem::Exists(metaFilePath))
            {
                AssetRegistry::Instance().UpdateMetaData(assetInfo, nullptr);
            }

            std::vector<Byte> fileData = FileSystem::ReadFromFile(metaFilePath, FileType::Binary);

            if (!binarySerialiser.Deserialise(fileData))
            {
                return;
            }

            AssetMetaData& metaData = RemoveConst(assetInfo)->MetaData;
            metaData.Deserialise(&binarySerialiser);
        }

        bool AssetPackage::AssetInfoValidate(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is null.");
                return false;
            }
            else if (!assetInfo->EnableMetaData)
            {
                IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo has MetaData disabled. No data is stored on disk for this asset.");
                return false;
            }
            else if (!assetInfo->IsValid())
            {
                IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is not valid.");
                return false;
            }
            else if (assetInfo->IsEngineFormat)
            {
                IS_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is an engine format.");
                return false;
            }
            return true;
        }

        IS_SERIALISABLE_CPP(AssetPackage);
    }
}