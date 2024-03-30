#pragma once

#include "Core/Singleton.h"

#include "Threading/SpinLock.h"


namespace Insight
{
    class IObject;

    struct ObjectItem
    {
        IObject* Object = nullptr;
        u64 NextFreeItem = 0;
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
        ObjectItem* m_objectItems = nullptr;
        u64 m_size = 0;
        u64 m_capacity = 0;
        mutable Threading::SpinLock m_objectSpinLock;
    };
}