#include "ECS/Entity.h"
#include "ECS/Entity.inl"

#include "ECS/ECSWorld.h"

#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/FreeCameraControllerComponent.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/TagComponent.h"

#include "World/WorldSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		void Entity::SetName(std::string name)
		{
			if (!IsVaild())
			{
				return;
			}
			ASSERT(m_entityManager);
			m_entityManager->GetEntityData(*this).Name = name;
		}

		bool Entity::IsVaild() const
		{
			return m_id != -1;
		}

		///bool Entity::HasComponent(u64 componentType) const
		///{
		///	if (!IsVaild())
		///	{
		///		return false;
		///	}
		///	ASSERT(m_entityManager)
		///	const EntityData& data = m_entityManager->GetEntityData(*this);
		///	return data.Components.find(componentType) != data.Components.end()l;
		///}

		std::string Entity::GetName() const
		{
			if (!IsVaild())
			{
				return "";
			}
			ASSERT(m_entityManager);
			return m_entityManager->GetEntityData(*this).Name;
		}

		int Entity::GetId() const
		{
			return m_id;
		}

		Core::GUID Entity::GetGuid() const
		{
			if (!IsVaild())
			{
				return Core::GUID::s_InvalidGUID;
			}
			ASSERT(m_entityManager); 
			return m_entityManager->GetEntityData(*this).GUID;
		}
#else 
		ComponentRegistryMap ComponentRegistry::m_registerFuncs;

		Component::Component()
			: m_allow_multiple(false)
			, m_removeable(true)
			, m_on_begin_called(false)
			, m_on_end_called(false)
		{ }

		Component::~Component()
		{ }

		IS_SERIALISABLE_CPP(Component)

		void ComponentRegistry::RegisterComponent(std::string_view componentType, std::function<Component*()> func)
		{
			if (auto itr = m_registerFuncs.find(std::string(componentType));
				itr == m_registerFuncs.end())
			{
				m_registerFuncs[std::string(componentType)] = std::move(func);
			}
		}

		void ComponentRegistry::UnregisterComponent(std::string_view componentType)
		{
			if (auto itr = m_registerFuncs.find(std::string(componentType));
				itr != m_registerFuncs.end())
			{
				m_registerFuncs.erase(itr);
			}
		}

		Component* ComponentRegistry::CreateComponent(std::string_view componentType)
		{
			if (auto itr = m_registerFuncs.find(std::string(componentType)); 
				itr != m_registerFuncs.end())
			{
				return itr->second();
			}
			IS_LOG_CORE_WARN("[ComponentRegistry::CreateComponent] ComponentType: '{0}', is unregistered.", componentType);
			return nullptr;
		}

		bool ComponentRegistry::HasComponent(std::string_view componentType)
		{
			return m_registerFuncs.find(std::string(componentType)) != m_registerFuncs.end();
		}

		std::vector<std::string> ComponentRegistry::GetComponentNames()
		{
			std::vector<std::string> componentTypeNames;
			componentTypeNames.reserve(m_registerFuncs.size());
			for (auto& [TypeName, CreateFunc] : m_registerFuncs)
			{
				componentTypeNames.push_back(TypeName);
			}

			// Sort the string in ascending order. Make strings all lower case to not worry about ASCII uppercase vs lowercase.
			std::sort(componentTypeNames.begin(), componentTypeNames.end(), [](std::string a, std::string b)
				{
					ToLower(a);
					ToLower(b);
					return a < b;
				});

			return componentTypeNames;
		}


#ifdef ECS_ENABLED
		Entity::Entity(ECSWorld* ecs_world)
			: m_ecs_world(ecs_world)
		{ }

		Entity::Entity(ECSWorld* ecs_world, std::string name)
			: m_ecs_world(ecs_world)
			, m_name(std::move(name))
		{ }
#else
		Entity::Entity(EntityManager* entityManager)
			: m_entityManager(entityManager)
		{ }

		Entity::Entity(EntityManager* entityManager, std::string name)
			: m_entityManager(entityManager)
			, m_name(std::move(name))
		{ }

		Entity::~Entity()
		{
			Destroy();
		}
#endif
		Ptr<Entity> Entity::AddChild()
		{
			return AddChild("Child");
		}

		Ptr<Entity> Entity::AddChild(std::string entityName)
		{
#ifdef ECS_ENABLED
			Ptr<Entity> entity = m_ecs_world->AddEntity(entityName);
#else
			Ptr<Entity> entity = m_entityManager->AddNewEntity(entityName);
#endif
			entity->m_parent = this;
			m_children.push_back(entity);
			return entity;
		}

		void Entity::RemoveChild(u32 index)
		{
			if (index >= m_children.size())
			{
				IS_LOG_CORE_WARN("[Entity::RemoveChild] No child entity at index '{}'.", index);
				return;
			}
			/// Remove the entity from the ecs world (delete the entity from memory)
			/// the remove it from this entity's child vector.
			std::vector<Ptr<Entity>>::iterator entityIter = m_children.begin() + index;
			Entity* entityPtr = entityIter->Get();
#ifdef ECS_ENABLED
			m_ecs_world->RemoveEntity(entityPtr);
#else
			m_entityManager->RemoveEntity(entityPtr);
#endif
			m_children.erase(entityIter);
		}

		u32 Entity::GetChildCount() const
		{
			return static_cast<u32>(m_children.size());
		}

		Ptr<Entity> Entity::GetFirstChild() const
		{
			if (m_children.size() > 0)
			{
				return m_children.at(0);
			}
			return nullptr;
		}

		Ptr<Entity> Entity::GetLastChild() const
		{
			if (m_children.size() > 0)
			{
				return m_children.back();
			}
			return nullptr;
		}

		Ptr<Entity> Entity::GetChild(u32 index) const
		{
			if (index >= m_children.size())
			{
				IS_LOG_CORE_WARN("[Entity::RemoveChild] No child entity at index '{}'.", index);
				return nullptr;
			}
			return m_children.at(index);
		}

		Component* Entity::AddComponentByName(std::string_view componentType)
		{
			Component* component = GetComponentByName(componentType);
			if (component && !component->m_allow_multiple)
			{
				IS_LOG_CORE_WARN("[Entity::AddComponentByName] Trying to add '{}'. ComponentType can not have multiple attached.", componentType);
				return component;
			}
			
			if (component == nullptr)
			{
				component = ComponentRegistry::CreateComponent(componentType);
				if (component)
				{
					component->m_ownerEntity = this;
					component->OnCreate();
					m_components.push_back(component);
				}
			}
			return component;
		}

		void Entity::RemoveComponent(const Component* component)
		{
			if (!component)
			{
				return;
			}
			RemoveComponent(component->GetGuid());
		}

		void Entity::RemoveComponent(const Core::GUID& guid)
		{
			RPtr<Component> componentToRemove;
			u32 index = 0;
			for (RPtr<Component>& component : m_components)
			{
				if (component->m_removeable
					&& component->GetGuid() == guid)
				{
					componentToRemove = std::move(component);
					m_components.erase(m_components.begin() + index);
					break;
				}
				++index;
			}

			if (!componentToRemove.IsValid())
			{
				IS_LOG_CORE_WARN("[Entity::RemoveComponentByGuid] Trying to remove a component which doesn't exists.");
				return;
			}

			componentToRemove->OnDestroy();
			componentToRemove.Reset();
		}

		void Entity::RemoveComponent(std::string_view componentType)
		{
			u32 index = 0;
			for (const RPtr<Component>& component : m_components)
			{
				if (component->m_removeable
					&& component->GetTypeName() == componentType)
				{
					break;
				}
				++index;
			}
			if (index < m_components.size())
			{
				(*(m_components.begin() + index))->OnDestroy();
				m_components.erase(m_components.begin() + index);
			}
			else
			{
				IS_LOG_CORE_INFO("[Entity::RemoveComponentByName] Tried to remove component '{}'. Component doesn't exists.", componentType);
			}
		}

		bool Entity::HasComponentByName(std::string_view componentType) const
		{
			for (RPtr<Component> const& component : m_components)
			{
				if (component->GetTypeName() == componentType)
				{
					return true;
				}
			}
			return false;
		}

		Component* Entity::GetComponentByGuid(const Core::GUID& guid) const
		{
			for (const RPtr<Component>& component : m_components)
			{
				if (component->GetGuid() == guid)
				{
					return component.Get();
				}
			}
			return nullptr;
		}


		Component* Entity::GetComponentByName(std::string_view componentType) const
		{
			for (const RPtr<Component>& component : m_components)
			{
				if (component->GetTypeName() == componentType) 
				{
					return component.Get();
				}
			}
			return nullptr;
		}

		Component* Entity::GetComponentByIndex(u32 idx) const
		{
			if (idx < static_cast<u32>(m_components.size()))
			{
				return m_components.at(static_cast<u64>(idx)).Get();
			}
			return nullptr;
		}

		u32 Entity::GetComponentCount() const
		{
			return static_cast<u32>(m_components.size());
		}

		bool Entity::IsEnabled() const
		{
			if (!m_isEnabled)
			{
				return false;
			}
			Entity* parentEntity = GetParent();
			while (parentEntity)
			{
				if (!parentEntity->IsEnabled())
				{
					return false;
				}
				parentEntity = parentEntity->GetParent();
			}
			return true;
		}

		void Entity::EarlyUpdate()
		{
			IS_PROFILE_FUNCTION();

			for (RPtr<Component>& component : m_components)
			{
				if (!component->m_on_begin_called)
				{
					component->m_on_begin_called = true;
					component->OnBegin();
				}
				component->OnEarlyUpdate();
			}
		}

		void Entity::Update(const float delta_time)
		{
			IS_PROFILE_FUNCTION();

			for (RPtr<Component>& component : m_components)
			{
				component->OnUpdate(delta_time);
			}
		}

		void Entity::LateUpdate()
		{
			IS_PROFILE_FUNCTION();

			for (RPtr<Component>& component : m_components)
			{
				if (!component->m_on_end_called)
				{
					component->m_on_end_called = true;
					component->OnEnd();
				}
				component->OnLateUpdate();
			}
		}

		void Entity::Destroy()
		{
			for (RPtr<Component>& component : m_components)
			{
				component->OnDestroy();
				component.Reset();
			}
			m_components.clear();
		}

		IS_SERIALISABLE_CPP(Entity)
#endif
	}
}