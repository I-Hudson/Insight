#include "Asset/Asset.h"
#include "Asset/AssetInfo.h"
#include "Core/Asserts.h"

namespace Insight::Runtime
{
    Asset::Asset(const AssetInfo* assetInfo)
        : m_assetInfo(assetInfo)
        , m_isMemoryAsset(assetInfo == nullptr)
    {
    }

    Asset::~Asset()
    {
        m_assetInfo = nullptr;
    }

    IS_SERIALISABLE_CPP(Asset);

    const AssetInfo* Asset::GetAssetInfo() const
    {
        ASSERT(!IsMemoryAsset());
        return m_assetInfo;
    }

    AssetState Asset::GetAssetState() const
    {
        return m_assetState;
    }

    bool Asset::IsMemoryAsset() const
    {
        return m_isMemoryAsset;
    }

    void Asset::SetName(const std::string& name)
    {
        m_name = name;
    }

    std::string Asset::GetName() const
    {
        return m_name;
    }

    std::string Asset::GetFileName() const
    {
        ASSERT(!IsMemoryAsset());
        return m_assetInfo->FileName;
    }

    std::string Asset::GetFilePath() const
    {
        ASSERT(!IsMemoryAsset());
       return m_assetInfo->GetFullFilePath();
    }

    std::string Asset::GetPackageName() const
    {
        ASSERT(!IsMemoryAsset());
        return m_assetInfo->PackageName;
    }

    std::string Asset::GetPackagePath() const
    {
        ASSERT(!IsMemoryAsset());
        return m_assetInfo->GetFullPackagePath();
    }

    void Asset::SerialiseHeader(Serialisation::ISerialiser* serialiser)
    {
        serialiser->Write("Version", m_header.Version);
        serialiser->Write("GUID", m_header.AssetGuid.ToString());
        serialiser->Write("AssetType", m_header.AssetType);
    }

    void Asset::DeserialiseHeader(Serialisation::ISerialiser* serialiser)
    {
        serialiser->Read("Version", m_header.Version);

        if (m_header.Version == 1)
        {
            std::string guidStr;
            serialiser->Read("GUID", guidStr);
            m_header.AssetGuid.StringToGuid(guidStr);
        }
        else if (m_header.Version == 2)
        {

        }
    }
}