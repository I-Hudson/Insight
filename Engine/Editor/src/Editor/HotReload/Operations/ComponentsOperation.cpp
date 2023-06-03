#include "Editor/HotReload/Operations/ComponentsOperation.h"
#include "Editor/HotReload/HotReloadSystem.h"

#include "World/WorldSystem.h"

namespace Insight::Editor
{
    ComponentsOperation::ComponentsOperation()
    {
    }

    ComponentsOperation::~ComponentsOperation()
    {
    }

    void ComponentsOperation::Reset()
    {
    }

    void ComponentsOperation::PreUnloadOperation()
    {
        const HotReloadLibrary& library = HotReloadSystem::Instance().GetLibrary();
        HotReloadMetaData metaData = library.GetMetaData();

        RemoveComponents(metaData);
    }

    void ComponentsOperation::PostLoadOperation()
    {
    }

    void ComponentsOperation::RemoveComponents(const HotReloadMetaData& metaData)
    {
        std::vector<TObjectPtr<Runtime::World>> worlds = Runtime::WorldSystem::Instance().GetAllWorlds();
        for (const TObjectPtr<Runtime::World>& world : worlds)
        {
            std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesFlatten();
            for (Ptr<ECS::Entity>& entity : entities)
            {
                for (const std::string& componentToRemove : metaData.ComponentNames)
                {
                    if (entity->HasComponentByName(componentToRemove))
                    {
                        entity->RemoveComponentByName(componentToRemove);
                    }
                }
            }
        }
    }

    void ComponentsOperation::AddComponents()
    {
    }
}