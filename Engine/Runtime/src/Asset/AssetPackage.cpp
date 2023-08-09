#include "Asset/AssetPackage.h"
#include "Asset/AssetRegistry.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"

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
            for (auto& [guid, assetInfo] : m_guidToAssetInfo)
            {
                Delete(assetInfo);
            }
            m_guidToAssetInfo.clear();
            m_pathToAssetGuid.clear();

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

            AssetInfo* assetInfo = New<AssetInfo>(path, m_packagePath, this);

            LoadMetaData(assetInfo);
            m_guidToAssetInfo[assetInfo->Guid] = assetInfo;
            m_pathToAssetGuid[assetInfo->GetFullFilePath()] = assetInfo->Guid;

            return assetInfo;
        }

        void AssetPackage::RemoveAsset(std::string_view path)
        {
            if (!HasAsset(path))
            {
                return;
            }

            Core::GUID assetGuid = GetGuidFromPath(path);

            if (auto iter = m_guidToAssetInfo.find(assetGuid);
                iter != m_guidToAssetInfo.end())
            {
                AssetInfo* assetInfo = iter->second;
                m_guidToAssetInfo.erase(assetInfo->Guid);
                m_pathToAssetGuid.erase(assetInfo->GetFullFilePath());
                Delete(assetInfo);
            }
        }

        void AssetPackage::RemoveAsset(const Core::GUID& guid)
        {
            if (!HasAsset(guid))
            {
                return;
            }

            if (auto iter = m_guidToAssetInfo.find(guid);
                iter != m_guidToAssetInfo.end())
            {
                AssetInfo* assetInfo = iter->second;
                m_guidToAssetInfo.erase(assetInfo->Guid);
                m_pathToAssetGuid.erase(assetInfo->GetFullFilePath());
                Delete(assetInfo);
            }
        }

        bool AssetPackage::HasAsset(std::string_view path) const
        {
            return HasAsset(GetGuidFromPath(path));
        }

        bool AssetPackage::HasAsset(const Core::GUID& guid) const
        {
            return m_guidToAssetInfo.find(guid) != m_guidToAssetInfo.end();
        }

        const AssetInfo* AssetPackage::GetAsset(std::string_view path) const
        {
            std::string formattedPath = std::string(path);
            FileSystem::PathToUnix(formattedPath);

            Core::GUID assetGuid = GetGuidFromPath(formattedPath);

            if (auto iter = m_guidToAssetInfo.find(assetGuid);
                iter != m_guidToAssetInfo.end())
            {
                return iter->second;
            }
            return nullptr;
        }

        const AssetInfo* AssetPackage::GetAsset(const Core::GUID& guid) const
        {
            if (auto iter = m_guidToAssetInfo.find(guid);
                iter != m_guidToAssetInfo.end())
            {
                return iter->second;
            }
            return nullptr;
        }

        void AssetPackage::ReplaceAsset(const AssetInfo* oldAsset, const AssetInfo* newAsset) const
        {
            auto oldIter = m_guidToAssetInfo.find(oldAsset->Guid);
            auto newIter = m_guidToAssetInfo.find(oldAsset->Guid);

            if (oldIter == m_guidToAssetInfo.end()
                || newIter != m_guidToAssetInfo.end())
            {
                return;
            }

            AssetInfo*& oldAssetInfo = RemoveConst(oldIter->second);
            oldAssetInfo = RemoveConst(newAsset);
        }

        std::vector<const AssetInfo*> AssetPackage::GetAllAssetInfos() const
        {
            std::vector<const AssetInfo*> assetInfos;
            for (const auto& [guid, info] : m_guidToAssetInfo)
            {
                assetInfos.push_back(info);
            }
            return assetInfos;
        }

        std::vector<Byte> AssetPackage::LoadAsset(std::string_view path) const
        {
            if (!HasAsset(path))
            {
                return {};
            }
            return LoadInteral(GetAsset(GetGuidFromPath(path)));
        }

        std::vector<Byte> AssetPackage::LoadAsset(Core::GUID guid) const
        {
            if (!HasAsset(guid))
            {
                return {};
            }
            auto iter = m_guidToAssetInfo.find(guid);
            return LoadInteral(iter->second);
        }

        void AssetPackage::BuildPackage(std::string_view path)
        {
            m_packagePath = path;
            Serialisation::BinarySerialiser serialiser(false);
            Serialise(&serialiser);
            FileSystem::SaveToFile(serialiser.GetRawData(), path, FileType::Binary, true);
        }

        std::vector<Byte> AssetPackage::LoadInteral(const AssetInfo* assetInfo) const
        {
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
            if (auto iter = m_pathToAssetGuid.find(std::string(path));
                iter != m_pathToAssetGuid.end())
            {
                return iter->second;
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