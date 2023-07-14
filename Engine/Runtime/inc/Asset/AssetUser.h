#pragma once

#include "Runtime/Defines.h"
#include "Asset/AssetInfo.h"

#include "Serialisation/ISerialisable.h"

namespace Insight::Runtime
{
    /// @brief 
    class IS_RUNTIME AssetUser : public Serialisation::ISerialisable
    {
    public:

        // Begin - ISerialisable -
        virtual void Serialise(::Insight::Serialisation::ISerialiser* serialiser) override { }
        virtual void Deserialise(::Insight::Serialisation::ISerialiser* serialiser) override { }
        // End - ISerialisable -

        void SetAssetInfo(AssetInfo* assetInfo) { m_assetInfo = assetInfo; }
        const AssetInfo* GetAssetInfo() const { return m_assetInfo; }

    private:
        AssetInfo* m_assetInfo;
    };
}