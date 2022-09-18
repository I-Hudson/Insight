#include "ECS/ECSWorld.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/TagComponent.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		ECSWorld::ECSWorld()
			: m_componentArrayManager(ComponentArrayManager(this))
		{ }

		ECSWorld::~ECSWorld()
		{
		}

		void ECSWorld::Update(float deltaTime)
		{
			m_componentArrayManager.Update(deltaTime);
		}

		Entity ECSWorld::AddEntity()
		{
			return m_entityManager.AddNewEntity();
		}

		void ECSWorld::RemoveEntity(Entity& entity)
		{
			m_entityManager.RemoveEntity(entity);
		}
		
		void ECSWorld::RemoveComponent(Entity entity, ComponentHandle& handle)
		{
			m_entityManager.RemoveComponentFromEntity(entity, handle);
			m_componentArrayManager.RemoveComponent(entity, handle);
		}
#else

		ECSWorld::ECSWorld()
		{
			ComponentRegistry::RegisterComponent(TransformComponent::Type_Name, []() { return NewTracked(TransformComponent); });
			ComponentRegistry::RegisterComponent(TagComponent::Type_Name, []() { return NewTracked(TagComponent); });
		}

		void ECSWorld::EarlyUpdate()
		{
			m_entityManager.EarlyUpdate();
		}

		void ECSWorld::Update(float deltaTime)
		{
			m_entityManager.Update(deltaTime);
		}

		void ECSWorld::LateUpdate()
		{
			m_entityManager.LateUpdate();
		}

		void ECSWorld::Destroy()
		{
			m_entityManager.Destroy();
		}

		Entity* ECSWorld::AddEntity()
		{
			return m_entityManager.AddNewEntity();
		}

		Entity* ECSWorld::AddEntity(std::string entity_name)
		{
			return m_entityManager.AddNewEntity(std::move(entity_name));
		}

		void ECSWorld::RemoveEntity(Entity*& entity)
		{
			m_entityManager.RemoveEntity(entity);
		}

		Ptr<Entity> ECSWorld::GetEntityByName(std::string_view entity_name) const
		{
			return m_entityManager.GetEntityByName(entity_name);
		}

		std::vector<Ptr<ECS::Entity>> ECSWorld::GetAllEntitiesWithComponentByName(std::string_view component_type) const
		{
			return m_entityManager.GetAllEntitiesWithComponentByName(component_type);
		}

#endif
	}
}