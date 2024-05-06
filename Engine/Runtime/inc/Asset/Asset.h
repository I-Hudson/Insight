#pragma once

#include "Asset/AssetInfo.h"
#include "Runtime/Defines.h"
#include "Core/IObject.h"
#include "Core/RefCount.h"

#include "Generated/Asset_reflect_generated.h"

namespace Insight::Runtime
{
    class AssetRegistry;

    enum class AssetState : u8
    {
        NotLoaded,
        Loading,
        Loaded,
        Unloaded,
        Unloading,
        Invalid
    };

    /// @brief Base class for asset classes. An Asset object is something which can comes from 
    /// a file on disk.
    REFLECT_CLASS();
    class IS_RUNTIME Asset : public Core::RefCount
    {
        REFLECT_GENERATED_BODY();
    public:
        Asset(const AssetInfo* assetInfo);
        virtual ~Asset() override;

        const AssetInfo* GetAssetInfo() const;
        AssetState GetAssetState() const;
        bool IsMemoryAsset() const;

        void SetName(const std::string& name);

        std::string GetName() const;
        std::string GetFileName() const;
        std::string GetFilePath() const;
        std::string GetPackageName() const;
        std::string GetPackagePath() const;

    protected:
        virtual void OnUnload() = 0;

    protected:
        std::string m_name;
        const AssetInfo* m_assetInfo = nullptr;
        std::atomic<AssetState> m_assetState = AssetState::NotLoaded;
        bool m_isMemoryAsset = false;

        friend class AssetRegistry;
    };
}