#include "Core/IObject.h"
#include "Core/ObjectManager.h"

namespace Insight
{
    IObject::IObject()
    {
        //ObjectManager::Instance().RegisterObject(this);
    }

    IObject::~IObject()
    {
        //ObjectManager::Instance().UnregisterObject(this);
    }

    IS_SERIALISABLE_CPP(IObject);

    void IObject::SetGuid(const Core::GUID& guid)
    {
        ObjectManager::Instance().UpdateGuidForObject(this, m_guid, guid);
        m_guid = guid;
    }

    Core::GUID IObject::GetGuid() const
    {
        return m_guid;
    }
}