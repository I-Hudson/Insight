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
}