#pragma once

#include "Core/TypeAlias.h"
#include "Core/Singleton.h"
#include "Core/GUID.h"

#include "Threading/SpinLock.h"

#include <unordered_set>
#include <unordered_map>
#include <string>

#define OBJECT_SORT_SET

#define OBJECT_DEBUG_INFO 1

namespace Insight
{
    class IObject;

    struct ObjectItem
    {
        IObject* Object = nullptr;
        u64 NextFreeItem = 0;

#if OBJECT_DEBUG_INFO
        std::string DebugName;
#endif
    };

    /// @brief Track all objects which have been created. This is a global tracker which can be
    /// queried from any where in the engine for an IObject.
    class IS_CORE ObjectManager : public Core::Singleton<ObjectManager>
    {
    public:
        ObjectManager();
        virtual ~ObjectManager() override;

        void RegisterObject(IObject* object);
        void UnregisterObject(IObject* object);

        bool HasObject(IObject* object) const;
        bool HasObject(const Core::GUID& guid) const;

        IObject* GetObject(const Core::GUID& guid) const;

    private:
#ifdef OBJECT_SORT_SET
        std::unordered_map<IObject*, ObjectItem*> m_objectItems;
        std::unordered_map<Core::GUID, ObjectItem*> m_guidToObjects;
#else
        ObjectItem* m_objectItems = nullptr;
        u64 m_size = 0;
        u64 m_capacity = 0;
#endif
        mutable Threading::SpinLock m_objectSpinLock;
    };
}