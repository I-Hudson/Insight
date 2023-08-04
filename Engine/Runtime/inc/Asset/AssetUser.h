#pragma once

#include "Runtime/Defines.h"
#include "Core/IObject.h"
#include "Asset/AssetInfo.h"

#include "Serialisation/ISerialisable.h"

namespace Insight::Runtime
{
    /// @brief 
    class IS_RUNTIME AssetUser : public IObject
    {
    public:
        virtual ~AssetUser() override
        {
            if (m_assetInfo)
            {
                RemoveConst(m_assetInfo)->Decr();
                RemoveConst(m_assetInfo)->MetaData.DependentGuids.erase(GetGuid());
            }
        }

        // Begin - ISerialisable -
        virtual void Serialise(::Insight::Serialisation::ISerialiser* serialiser) override { }
        virtual void Deserialise(::Insight::Serialisation::ISerialiser* serialiser) override { }
        // End - ISerialisable -

        void SetAssetInfo(const AssetInfo* assetInfo) 
        {
            if (m_assetInfo)
            {
                RemoveConst(m_assetInfo)->Decr();
                RemoveConst(m_assetInfo)->MetaData.DependentGuids.erase(GetGuid());
            }
            m_assetInfo = assetInfo; 
            RemoveConst(m_assetInfo)->Incr();
            RemoveConst(m_assetInfo)->MetaData.DependentGuids.insert(GetGuid());
        }
        const AssetInfo* GetAssetInfo() const { return m_assetInfo; }

    private:
        const AssetInfo* m_assetInfo = nullptr;
    };
}