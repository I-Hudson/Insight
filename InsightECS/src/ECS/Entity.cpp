#include "ECS/Entity.h"
#include "ECS/ECSWorld.h"

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
		ComponentRegistryMap ComponentRegistry::m_register_funcs;

		Component::Component()
			: m_allow_multiple(false)
			, m_removeable(true)
			, m_on_begin_called(false)
			, m_on_end_called(0)
		{ }

		Component::~Component()
		{ }

		void ComponentRegistry::RegisterComponent(std::string_view component_type, std::function<Component*()> func)
		{
			if (auto itr = m_register_funcs.find(std::string(component_type));
				itr != m_register_funcs.end())
			{
				IS_CORE_ERROR("[ComponentRegistry::RegisterComponent] ComponentType: '{0}' is already registered.", component_type);
				return;
			}
			m_register_funcs[std::string(component_type)] = std::move(func);
		}

		Component* ComponentRegistry::CreateComponent(std::string_view component_type)
		{
			if (auto itr = m_register_funcs.find(std::string(component_type)); 
				itr != m_register_funcs.end())
			{
				return itr->second();
			}
			IS_CORE_ERROR("[ComponentRegistry::CreateComponent] ComponentType: '{0}', is unregistered.", component_type);
			return nullptr;
		}


		Entity::Entity(ECSWorld* ecs_world)
			: m_ecsWorld(ecs_world)
		{ }

		Entity::Entity(ECSWorld* ecs_world, std::string name)
			: m_ecsWorld(ecs_world)
			, m_name(std::move(name))
		{ }

		Ptr<Entity> Entity::AddChild()
		{
			return AddChild("Child");
		}

		Ptr<Entity> Entity::AddChild(std::string entity_name)
		{
			Ptr<Entity> entity = m_ecsWorld->AddEntity(entity_name);
			m_children.push_back(entity);
			return entity;
		}

		void Entity::RemoveChild(u32 index)
		{
			if (index >= m_children.size())
			{
				IS_CORE_WARN("[Entity::RemoveChild] No child entity at index '{}'.", index);
				return;
			}
			/// Remove the entity from the ecs world (delete the entity from memory)
			/// the remove it from this entity's child vector.
			std::vector<Ptr<Entity>>::iterator entity_itr = m_children.begin() + index;
			Entity* entity_ptr = entity_itr->Get();
			m_ecsWorld->RemoveEntity(entity_ptr);
			m_children.erase(entity_itr);
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
				IS_CORE_WARN("[Entity::RemoveChild] No child entity at index '{}'.", index);
				return nullptr;
			}
			return m_children.at(index);
		}

		Component* Entity::AddComponentByName(std::string_view component_type)
		{
			Component* component = GetComponentByName(component_type);
			if (component && !component->m_allow_multiple)
			{
				IS_CORE_WARN("[Entity::AddComponentByName] Trying to add '{}'. ComponentType can not have multiple attached.", component_type);
				return component;
			}
			
			if (component == nullptr)
			{
				component = ComponentRegistry::CreateComponent(component_type);
				component->OnCreate();
				m_components.push_back(component);
			}
			return component;
		}

		Component* Entity::GetComponentByName(std::string_view component_type) const
		{
			for (const RPtr<Component>& component : m_components)
			{
				if (component->GetTypeName() == component_type) 
				{
					return component.Get();
				}
			}
			return nullptr;
		}

		void Entity::RemoveComponentByName(std::string_view component_type)
		{
			u32 index = 0;
			for (const RPtr<Component>& component : m_components)
			{
				if (component->m_removeable
					&& component->GetTypeName() == component_type)
				{
					break;
				}
				++index;
			}
			if (index < m_components.size())
			{
				m_components.erase(m_components.begin() + index);
			}
			else
			{
				IS_CORE_INFO("[Entity::RemoveComponentByName] Tried to remove component '{}'. Component doesn't exists.", component_type);
			}
		}

		void Entity::EarlyUpdate()
		{
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
			for (RPtr<Component>& component : m_components)
			{
				component->OnUpdate(delta_time);
			}
		}

		void Entity::LateUpdate()
		{
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
#endif
	}
}