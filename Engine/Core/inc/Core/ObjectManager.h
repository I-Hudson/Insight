#pragma once

#include "Core/Singleton.h"

#include <unordered_map>

namespace Insight
{
    class IObject;

    /// @brief Track all objects which have been created. This is a global tracker which can be
    /// queried from any where in the engine for an IObject.
    class IS_CORE ObjectManager : public Core::Singleton<ObjectManager>
    {
    public:
        ObjectManager();
        virtual ~ObjectManager() override;

        void RegisterObject(IObject* object);
        void UnregisterObject(IObject* object);

        void UpdateGuidForObject(IObject* object, const Core::GUID& oldGuid, const Core::GUID& newGuid);

        bool HasObject(IObject* object) const;
        bool HasObject(const Core::GUID& guid) const;

    private:
        std::unordered_map<Core::GUID, IObject*> m_guidToObjects;
    };
}