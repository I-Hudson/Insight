#include "Core/ObjectManager.h"

namespace Insight
{
    ObjectManager::ObjectManager()
    {
    }

    ObjectManager::~ObjectManager()
    {
    }

    void ObjectManager::RegisterObject(IObject* object)
    {
    }

    void ObjectManager::UnregisterObject(IObject* object)
    {
    }

    void ObjectManager::UpdateGuidForObject(IObject* object, const Core::GUID& oldGuid, const Core::GUID& newGuid)
    {
    }

    bool ObjectManager::HasObject(IObject* object) const
    {
        return false;
    }

    bool ObjectManager::HasObject(const Core::GUID& guid) const
    {
        return false;
    }
}