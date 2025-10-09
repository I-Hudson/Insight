#include "Core/IObject.h"
#include "Core/ObjectManager.h"

namespace Insight
{
    IObject::IObject()
    {
        ObjectManager::Instance().RegisterObject(this);
    }

    IObject::~IObject()
    {
        ObjectManager::Instance().UnregisterObject(this);
    }

    IS_SERIALISABLE_CPP(IObject);

    void IObject::SetGuid(const Core::GUID& guid)
    {
        ObjectManager::Instance().UnregisterObject(this);
        m_guid = guid;
        ObjectManager::Instance().RegisterObject(this);
    }

    Core::GUID IObject::GetGuid() const
    {
        return m_guid;
    }
}