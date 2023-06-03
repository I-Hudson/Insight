#pragma once

#include "Editor/HotReload/HotReloadOperation.h"
#include "Editor/HotReload/HotReloadMetaData.h"

#include "ECS/Entity.h"


namespace Insight::Runtime
{
    class World;
}

namespace Insight::Editor
{
    class ComponentsOperation : public HotReloadOperation
    {
        struct ComponentReference
        {
            std::string TypeName;
            Core::GUID Guid;
            std::vector<Core::GUID> ComponentReferences; // Which other components does this one reference.
        };
        struct EntityReference
        {
            EntityReference() { }
            EntityReference(Runtime::World* world, Core::GUID guid)
                : World(world), Guid(std::move(guid))
            { }

            ComponentReference& AddComponentToRelink(ECS::Component* component);
            ComponentReference& AddComponentToRemove(ECS::Component* component);

            Runtime::World* World;
            Core::GUID Guid;
            std::unordered_map<Core::GUID, ComponentReference> ComponentsToRelink;
            std::unordered_map<Core::GUID, ComponentReference> ComponentsToRemove;
        };

    public:
        ComponentsOperation();
        virtual ~ComponentsOperation() override;

        virtual void Reset() override;
        virtual void PreUnloadOperation() override;
        virtual void PostLoadOperation() override;

    private:
        EntityReference& AddEntityReference(Runtime::World* world, const Core::GUID& guid);
        EntityReference& GetEntityReference(const Core::GUID& guid);
        
        void FindAllComponents(const HotReloadMetaData& metaData);

        /// @brief Remove all components which exist in the project DLL.
        /// @param metaData 
        void RemoveComponents(const HotReloadMetaData& metaData);

        /// @brief Add project dll components back to the entities they were on.
        void AddComponents();

    private:
        std::unordered_map<Core::GUID, EntityReference> m_entities;
    };
}