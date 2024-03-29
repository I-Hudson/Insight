#pragma once

#include "Runtime/Defines.h"
#include "ECS/EntityManager.h"
#include "ECS/ComponentArray.h"

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		///using ECSCommandBufferCmd = std::function<void()>;
		/////// <summary>
		/////// Buffer to store all commands which have been queued to be executed next 
		/////// frame.
		/////// </summary>
		///class ECSCommandBuffer
		///{
		///public:

		///	void Pump();

		///	Entity AddEntity();
		///	void RemoveEntity(Entity& entity);

		///	template<typename Component>
		///	ComponentHandle AddComponent(Entity entity)
		///	{
		///		ComponentHandle handle = m_componentArrayManager.AddComponent<Component>(entity);
		///		m_entityManager.AddComponentToEntity(entity, handle);
		///		return handle;
		///	}
		///	void RemoveComponent(Entity entity, ComponentHandle& handle);

		///private:

		///	/// This is per frame data and should be added back to ECSWorld at the end of frame.
		///	/// Think of anything in these as queued.
		///	EntityManager m_entityManager;
		///	ComponentArrayManager m_componentArrayManager;
		///};

		//// <summary>
		//// World manager for all ECS objects (entities and components).
		//// This should have creation, deletion, and updating all objects.
		//// </summary>
		class IS_RUNTIME ECSWorld
		{
		public:
			ECSWorld();
			~ECSWorld();

			void Update(float deltaTime);

			Entity AddEntity();
			void RemoveEntity(Entity& entity);

			template<typename Component>
			ComponentHandle AddComponent(Entity entity)
			{
				ComponentHandle handle = m_componentArrayManager.AddComponent<Component>(entity);
				m_entityManager.AddComponentToEntity(entity, handle);
				return handle;
			}
			void RemoveComponent(Entity entity, ComponentHandle& handle);

		private:
			/// m_entityManager and m_componentArrayManager are our true world entites and data.
			EntityManager m_entityManager;
			ComponentArrayManager m_componentArrayManager;
		};
#else 

		class IS_RUNTIME ECSWorld
		{
		public:
			ECSWorld();

			Entity* AddEntity();
			Entity* AddEntity(std::string entity_name);
			void RemoveEntity(Entity*& entity);

			void EarlyUpdate();
			void Update(const float deltaTime);
			void LateUpdate();

			void Destroy();

			Ptr<Entity> GetEntityByName(std::string_view entity_name) const;
			std::vector<Ptr<ECS::Entity>> GetAllEntitiesWithComponentByName(std::string_view component_type) const;

		private:
			std::vector<Ptr<Entity>> m_root_entities;
			EntityManager m_entityManager;
		};

#endif
	}
}