#include "Core/ObjectManager.h"
#include "Core/IObject.h"

#include "Core/Memory.h"
#include "Core/Logger.h"
#undef GetObject

#include "Threading/ScopedLock.h"

namespace Insight
{
    ObjectManager::ObjectManager()
    {
    }

    ObjectManager::~ObjectManager()
    {
        if (m_size > 0)
        {
            IS_CORE_ERROR("[ObjectManager::~ObjectManager] Not all objects are unregistered.");
        }
     
        if (m_objectItems)
        {
            ::DeleteBytes(m_objectItems);
        }
    }

    void ObjectManager::RegisterObject(IObject* object)
    {
        if (object == nullptr)
        {
            return;
        }

        if (m_size == m_capacity)
        {
            const u64 newCapacity = m_capacity != 0 ? m_size * 2 : 128;
            m_objectItems = static_cast<ObjectItem*>(::ReallocBytes(m_objectItems, sizeof(m_objectItems[0]) * newCapacity));
            ASSERT(m_objectItems != nullptr);
            for (size_t i = m_capacity; i < newCapacity; ++i)
            {
                ObjectItem& item = m_objectItems[i];
                item.Object = nullptr;
                item.NextFreeItem = i;
            }
            m_capacity = newCapacity;
        }

        Threading::ScopedLock scopedLock(m_objectSpinLock);
        const u64 nextItem = m_objectItems[m_size].NextFreeItem;

        ObjectItem& item = m_objectItems[nextItem];
        ASSERT(item.Object == nullptr);
        item.Object = object;
        object->m_objectIndex = m_size;
        ++m_size;
    }

    void ObjectManager::UnregisterObject(IObject* object)
    {
        if (object == nullptr)
        {
            return;
        }

        const u64 objectIndex = object->m_objectIndex;
        Threading::ScopedLock scopedLock(m_objectSpinLock);

        ASSERT_MSG(objectIndex < m_capacity, "[ObjectManager::UnregisterObject] Out of range.");
        --m_size;
        m_objectItems[m_size].NextFreeItem = objectIndex;

        ObjectItem& item = m_objectItems[objectIndex];
        item.Object = nullptr;
    }

    bool ObjectManager::HasObject(IObject* object) const
    {
        if (object == nullptr)
        {
            return false;
        }

        Threading::ScopedLock scopedLock(m_objectSpinLock);
        const u64 objectIndex = object->m_objectIndex;
        ASSERT_MSG(objectIndex < m_size, "[ObjectManager::UnregisterObject] Out of range.");

        return m_objectItems[objectIndex].Object != nullptr;
    }

    bool ObjectManager::HasObject(const Core::GUID& guid) const
    {
        if (!guid.IsValid())
        {
            return false;
        }

        Threading::ScopedLock scopedLock(m_objectSpinLock);
        for (size_t i = 0; i < m_size; ++i)
        {
            if (m_objectItems[i].Object
                && m_objectItems[i].Object->GetGuid() == guid)
            {
                return true;
            }
        }
        return false;
    }

    IObject* ObjectManager::GetObject(const Core::GUID& guid) const
    {
        if (!guid.IsValid())
        {
            return nullptr;
        }

        Threading::ScopedLock scopedLock(m_objectSpinLock);
        for (size_t i = 0; i < m_size; ++i)
        {
            if (m_objectItems[i].Object
                && m_objectItems[i].Object->GetGuid() == guid)
            {
                return m_objectItems[i].Object;
            }
        }
        return nullptr;
    }
}