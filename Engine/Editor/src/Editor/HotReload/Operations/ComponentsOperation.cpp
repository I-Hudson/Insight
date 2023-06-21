#include "Editor/HotReload/Operations/ComponentsOperation.h"
#include "Editor/HotReload/HotReloadSystem.h"
#include "Editor/Defines.h"

#include "World/WorldSystem.h"

#include "Core/Logger.h"
#include "Algorithm/Vector.h"

namespace Insight::Editor
{
    ComponentsOperation::ComponentReference& ComponentsOperation::EntityReference::AddComponentToRelink(ECS::Component* component)
    {
        ASSERT(component);
        if (auto iter = ComponentsToRelink.find(component->GetGuid()); iter != ComponentsToRelink.end())
        {
            return iter->second;
        }

        ComponentsOperation::ComponentReference& ref = ComponentsToRelink[component->GetGuid()];
        ref.TypeName = component->GetTypeName();
        ref.Guid = component->GetGuid();
        return ref;
    }

    ComponentsOperation::ComponentReference& ComponentsOperation::EntityReference::AddComponentToRemove(ECS::Component* component)
    {
        ASSERT(component);
        if (auto iter = ComponentsToRemove.find(component->GetGuid()); iter != ComponentsToRemove.end())
        {
            return iter->second;
        }

        ComponentsOperation::ComponentReference& ref = ComponentsToRemove[component->GetGuid()];
        ref.TypeName = component->GetTypeName();
        ref.Guid = component->GetGuid();
        return ref;
    }

    ComponentsOperation::ComponentsOperation()
    {
    }

    ComponentsOperation::~ComponentsOperation()
    {
    }

    void ComponentsOperation::Reset()
    {
        m_entities.clear();
    }

    void ComponentsOperation::PreUnloadOperation()
    {
        const HotReloadLibrary& library = HotReloadSystem::Instance().GetLibrary();
        HotReloadMetaData metaData = library.GetMetaData();

        FindAllComponents(metaData);
        RemoveComponents(metaData);
    }

    void ComponentsOperation::PostLoadOperation()
    {
        AddComponents();
    }

    ComponentsOperation::EntityReference& ComponentsOperation::AddEntityReference(Runtime::World* world, const Core::GUID& guid)
    {
        if (auto iter = m_entities.find(guid); iter != m_entities.end())
        {
            return iter->second;
        }
        return m_entities[guid] = ComponentsOperation::EntityReference(world, guid);
    }

    ComponentsOperation::EntityReference& ComponentsOperation::GetEntityReference(const Core::GUID& guid)
    {
        if (auto iter = m_entities.find(guid); iter != m_entities.end())
        {
            return iter->second;
        }
        FAIL_ASSERT();
        return m_entities.begin()->second;
    }

    void ComponentsOperation::FindAllComponents(const HotReloadMetaData& metaData)
    {
        std::vector<TObjectPtr<Runtime::World>> worlds = Runtime::WorldSystem::Instance().GetAllWorlds();
        for (const TObjectPtr<Runtime::World>& world : worlds)
        {
            std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesFlatten();
            for (Ptr<ECS::Entity>& entity : entities)
            {
                for (u32 i = 0; i < entity->GetComponentCount(); ++i)
                {
                    ECS::Component* component = entity->GetComponentByIndex(i);
                    if (Algorithm::VectorContains(metaData.ComponentNames, component->GetTypeName()))
                    {
                        // The component being evaluated with from the project dll. So it should always be removed.
                        ComponentsOperation::EntityReference& entityReference = AddEntityReference(world.Get(), entity->GetGUID());
                        ComponentsOperation::ComponentReference& componentReference = entityReference.AddComponentToRemove(component);
                    }

                    Reflect::TypeInfo typeInfo = component->GetTypeInfo();
                    for (const Reflect::MemberInfo& member : typeInfo.GetMemberInfos())
                    {
                        // Check if this member is of a project dll type, if it is and is visible to the editor then it could being set 
                        // in the editor so will need relinking after the dll has been reloaded.
                        if (Algorithm::VectorContains(metaData.ComponentNames, member.GetTypeId().GetTypeName())
                            && member.HasFlag("EditorVisible"))
                        {
                            if (member.GetValueType() != Reflect::EReflectValueType::Pointer)
                            {
                                IS_CORE_WARN("[ComponentsOperation::FindAllComponents] Member '{0}::{1}' is value type '{2}'. Linking only works with pointers."
                                    , typeInfo.GetTypeId().GetTypeName().data(), member.GetMemberName().data(), (int)member.GetType().GetTypeName().data());
                                continue;
                            }

                            ComponentsOperation::EntityReference& entityReference = AddEntityReference(world.Get(), entity->GetGUID());
                            ComponentsOperation::ComponentReference& componentReference = entityReference.AddComponentToRelink(component);

                            /* 
                            As the GetData func only returns a void*, if the member is a pointer itself
                            then GetData will return a pointer to the member pointer's.
                            Because of this we need to do a hack to cast the member pointer to a pointer pointer.
                            Then we can get the value of the member's pointer. This needs looking at in the Reflect library.
                            The Reflect library needs some kind of GetPointer function to handle this.
                            */
                            void* memberPointer = member.GetMemberPointer();
                            ECS::Component** memberComponentPointer = reinterpret_cast<ECS::Component**>(memberPointer);
                            ECS::Component*& memberComponent = *memberComponentPointer;

                            if (memberComponent)
                            {
                                componentReference.ComponentReferences.push_back(memberComponent->GetGuid());
                            }
                        }
                    }
                }
            }
        }
    }

    void ComponentsOperation::RemoveComponents(const HotReloadMetaData& metaData)
    {
        for (const auto& [entityGuid, entity] : m_entities)
        {
            ECS::Entity* ecsEntity = entity.World->GetEntityByGUID(entityGuid);
            if (!ecsEntity)
            {
                IS_CORE_ERROR("[ComponentsOperation::RemoveComponents] Unable to find Entity with guid: '{0}'", entityGuid.ToString());
                continue;
            }

            for (const auto& [componentGuid, component] : entity.ComponentsToRemove)
            {
                ecsEntity->RemoveComponent(component.TypeName);
            }
        }
    }

    void ComponentsOperation::AddComponents()
    {
        for (const auto& [entityGuid, entity] : m_entities)
        {
            ECS::Entity* ecsEntity = entity.World->GetEntityByGUID(entityGuid);
            if (!ecsEntity)
            {
                IS_CORE_ERROR("[ComponentsOperation::AddComponents] Unable to find Entity with guid: '{0}'", entityGuid.ToString());
                continue;
            }

            for (const auto& [componentGuid, component] : entity.ComponentsToRemove)
            {
                ECS::Component* addedComponent = ecsEntity->AddComponentByName(component.TypeName);
                if (addedComponent)
                {
                    addedComponent->m_guid = componentGuid;
                }
            }
        }
    }

}