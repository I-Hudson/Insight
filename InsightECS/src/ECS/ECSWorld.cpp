#include "ECS/ECSWorld.h"

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
		{ }

		void ECSWorld::Update(float deltaTime)
		{
			m_entityManager.Update(deltaTime);
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

#endif
	}
}