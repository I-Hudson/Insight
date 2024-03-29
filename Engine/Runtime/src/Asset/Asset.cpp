#include "Asset/Asset.h"
#include "Asset/AssetInfo.h"

namespace Insight::Runtime
{
    Asset::Asset(const AssetInfo* assetInfo)
        : m_assetInfo(assetInfo)
        , m_isMemoryAsset(m_assetInfo != nullptr)
    {
    }

    Asset::~Asset()
    {
        m_assetInfo = nullptr;
    }

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
}