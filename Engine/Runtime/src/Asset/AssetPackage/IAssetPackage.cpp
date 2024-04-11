#include "Asset/AssetPackage/IAssetPackage.h"
#include "Asset/AssetPackage/AssetPackageZip.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetInfo.h"

#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Core/Logger.h"
#include "FileSystem/FileSystem.h"

#include "Algorithm/Vector.h"

namespace Insight
{
    namespace Runtime
    {
        IAssetPackage::IAssetPackage(std::string_view packagePath, std::string_view packageName, AssetPackageType packageType)
        {
            m_packagePath = FileSystem::GetAbsolutePath(packagePath);
            m_packageName = packageName;
            m_packageType = packageType;
        }

        IAssetPackage::~IAssetPackage()
        {
            std::vector<AssetInfo*> infos = m_assetInfos;
            for (int infoIdx = 0; infoIdx < infos.size(); ++infoIdx)
            {
                AssetRegistry::Instance().RemoveAsset(infos.at(infoIdx)->GetFullFilePath());
            }
            m_assetInfos.clear();
        }

        std::string_view IAssetPackage::GetPath() const
        {
            std::lock_guard lock(m_packageLock);
            return m_packagePath;
        }

        std::string_view IAssetPackage::GetName() const
        {
            std::lock_guard lock(m_packageLock);
            return m_packageName;
        }

        AssetPackageType IAssetPackage::GetPackageType() const
        {
            std::lock_guard lock(m_packageLock);
            return m_packageType;
        }

        const AssetInfo* IAssetPackage::AddAsset(std::string_view path)
        {
            if (HasAsset(path))
            {
                IS_LOG_CORE_INFO("[IAssetPackage::AddAsset] Asset at path '{}' already added to this package.", path.data());
                return GetAsset(path);
            }

            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(path));
            return AddAsset(assetInfo);
        }

        const AssetInfo* IAssetPackage::AddAsset(AssetInfo* assetInfo)
        {
            if (assetInfo && !HasAsset(assetInfo))
            {
                assetInfo->AssetPackage->RemoveAsset(assetInfo);
                assetInfo->AssetPackage = this;
                assetInfo->PackageName = m_packageName;
                assetInfo->PackagePath = m_packagePath;

                std::lock_guard lock(m_packageLock);
                m_assetInfos.push_back(assetInfo);
                return assetInfo;
            }
            else
            {
                return GetAsset(assetInfo->Guid);
            }
        }

        void IAssetPackage::RemoveAsset(std::string_view path)
        {
            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(path));
            RemoveAsset(assetInfo);
        }

        void IAssetPackage::RemoveAsset(const Core::GUID& guid)
        {
            AssetInfo* assetInfo = RemoveConst(AssetRegistry::Instance().GetAsset(guid));
            RemoveAsset(assetInfo);
        }

        void IAssetPackage::RemoveAsset(AssetInfo* assetInfo)
        {
            if (!assetInfo || !HasAsset(assetInfo->Guid))
            {
                return;
            }
            std::lock_guard lock(m_packageLock);
            assetInfo->AssetPackage = nullptr;
            assetInfo->PackageName.clear();
            assetInfo->PackagePath.clear();
            ASSERT(Algorithm::VectorRemove(m_assetInfos, assetInfo));
        }

        bool IAssetPackage::HasAsset(std::string_view path) const
        {
            return HasAsset(GetGuidFromPath(path));
        }

        bool IAssetPackage::HasAsset(const Core::GUID& guid) const
        {
            std::lock_guard lock(m_packageLock);
            return Algorithm::VectorContainsIf(m_assetInfos, [&guid](const AssetInfo* assetInfo)
                {
                    if (!assetInfo)
                    {
                        return false;
                    }
                    return guid == assetInfo->Guid;
                });
        }

        bool IAssetPackage::HasAsset(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                return nullptr;
            }
            return HasAsset(assetInfo->Guid);
        }

        const AssetInfo* IAssetPackage::GetAsset(std::string_view path) const
        {
            std::string formattedPath = std::string(path);
            FileSystem::PathToUnix(formattedPath);

            Core::GUID assetGuid = GetGuidFromPath(formattedPath);
            return GetAsset(assetGuid);
        }

        const AssetInfo* IAssetPackage::GetAsset(const Core::GUID& guid) const
        {
            std::lock_guard lock(m_packageLock);
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

        const AssetInfo* IAssetPackage::GetAsset(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                return nullptr;
            }
            return GetAsset(assetInfo->Guid);
        }

        void IAssetPackage::ReplaceAsset(AssetInfo* oldAsset, AssetInfo* newAsset)
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

        const std::vector<AssetInfo*>&IAssetPackage::GetAllAssetInfos() const
        {
            return m_assetInfos;
        }

        std::vector<Byte> IAssetPackage::LoadAsset(std::string_view path) const
        {
            const AssetInfo* assetInfo = GetAsset(GetGuidFromPath(path));
            if (!assetInfo)
            {
                return {};
            }
            return LoadInteral(assetInfo);
        }

        std::vector<Byte> IAssetPackage::LoadAsset(Core::GUID guid) const
        {
            const AssetInfo* assetInfo = GetAsset(guid);
            if (!assetInfo)
            {
                return {};
            }
            return LoadInteral(assetInfo);
        }

        void IAssetPackage::BuildPackage(std::string_view path)
        {
            std::lock_guard lock(m_packageLock);

            AssetPackageZip buildPackage(m_packagePath, m_packageName);
            buildPackage.m_assetInfos = m_assetInfos;

            buildPackage.BuildPackage(path);

            buildPackage.m_assetInfos.clear();
        }

        Core::GUID IAssetPackage::GetGuidFromPath(std::string_view path) const
        {
            std::lock_guard lock(m_packageLock);
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

        void IAssetPackage::LoadMetaData(AssetInfo* assetInfo)
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

            AssetMetaData* metaData = RemoveConst(assetInfo)->MetaData;
            metaData->Deserialise(&binarySerialiser);
        }

        bool IAssetPackage::AssetInfoValidate(const AssetInfo* assetInfo) const
        {
            if (!assetInfo)
            {
                IS_LOG_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is null.");
                return false;
            }
            else if (!assetInfo->EnableMetaData)
            {
                IS_LOG_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo has MetaData disabled. No data is stored on disk for this asset.");
                return false;
            }
            else if (!assetInfo->IsValid())
            {
                IS_LOG_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is not valid.");
                return false;
            }
            else if (assetInfo->IsEngineFormat)
            {
                IS_LOG_CORE_WARN("[AssetRegistry::UpdateMetaData] AssetInfo is an engine format.");
                return false;
            }
            return true;
        }
    }
}