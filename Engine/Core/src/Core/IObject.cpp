#include "Core/IObject.h"

namespace Insight
{
    IObject::IObject()
    {
    }

    IObject::~IObject()
    {
    }

    IS_SERIALISABLE_CPP(IObject);

    void IObject::SetGuid(Core::GUID guid)
    {
        m_guid = std::move(guid);
    }

    Core::GUID IObject::GetGuid() const
    {
        return m_guid;
    }
}