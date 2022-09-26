#include "ECS/ECS.h"

#include "ECS/ExampleComponent.h"

#include "Core/Profiler.h"

#ifdef IS_EXP_ECS

namespace Insight
{
	namespace App
	{
		ECS::ECS()
		{
			RegisterComponent<ExampleComponent>();
			RegisterComponent<ExampleComponentPrint>();
			RegisterComponent<TransformComponent>();
			RegisterComponent<TagComponent>();

			Entity e = AddEntity();
			AddComponent<TransformComponent>(e);
			for (size_t i = 0; i < 10000000; i++)
			{
				AddComponent<ExampleComponent>(e);
			}
			AddComponent<ExampleComponentPrint>(e);
			AddComponent<TagComponent>(e);

			IS_CORE_INFO("Entity has TransformComponent: {}", m_entityManager.HasComponent(e, ComponentTypeToId<TransformComponent>()));
			IS_CORE_INFO("Entity component count: {}", m_entityManager.GetComponentCount(e));

			RemoveComponent<TransformComponent>(e);
			IS_CORE_INFO("Entity has TransformComponent: {}", m_entityManager.HasComponent(e, ComponentTypeToId<TransformComponent>()));
		}

		Entity ECS::GetOwnerEntity(ComponentHandle componentHandle)
		{
			IECSComponentArray* componentArray = GetComponentArray(componentHandle);
			return componentArray->GetOwnerEntity(componentHandle);
		}

		void ECS::Update(float deltaTime)
		{
			IS_PROFILE_FUNCTION();

			UpdateRequests();
			PrepareFrameUpdateData();

			for (size_t i = 0; i < m_componentSystemsToUpdate.size(); ++i)
			{
				const u64 componentId = m_componentSystemsToUpdate.at(i);
				IECSSystem* system = m_componentSystems.find(componentId)->second.Get();
				IECSComponentArray* componentArray = m_componentArrays.find(componentId)->second.Get();

				ASSERT_MSG(system && componentArray, "[ECS::Update] Both the system and component array must be valid.");
				{
					IS_PROFILE_SCOPE("Component system update");
					system->Update(*this, componentArray, deltaTime);
				}
			}
		}

		void ECS::UpdateRequests()
		{
			IS_PROFILE_FUNCTION();
			for (size_t i = 0; i < m_requests.AddRequests.size(); ++i)
			{
				m_requests.AddRequests.at(i)();
			}
			m_requests.AddRequests.clear();

			for (size_t i = 0; i < m_requests.RemoveRequests.size(); ++i)
			{
				m_requests.RemoveRequests.at(i)();
			}
			m_requests.RemoveRequests.clear();
		}

		void ECS::PrepareFrameUpdateData()
		{
			IS_PROFILE_FUNCTION();
			for (size_t i = 0; i < m_componentSystemsToUpdate.size(); ++i)
			{

			}
		}

		////--------------------------------------------------------------------------------------------

		Entity ECSEntityManager::AddEntity()
		{
			if (m_freeEntities.size() == 0)
			{
				m_entities.push_back(EntityData());
				m_freeEntities.insert((int)m_entities.size() - 1);
			}

			ASSERT(m_freeEntities.size() > 0);
			Entity e = *m_freeEntities.begin();
			m_freeEntities.erase(e);
			return e;
		}

		void ECSEntityManager::RemoveEntity(Entity& entity)
		{
			m_freeEntities.insert(entity);
			entity = -1;
		}

		void ECSEntityManager::AddComponentToEntity(Entity entity, ComponentHandle component)
		{
			if (!IsValidEntity(entity))
			{
				return;
			}
			EntityData& data = GetEntityData(entity);
			data.Components[component.GetTypeId()].push_back(component.GetId());
		}

		void ECSEntityManager::RemoveComponentFromEntity(Entity entity, ComponentHandle component)
		{
			if (!IsValidEntity(entity))
			{
				return;
			}
			EntityData& data = GetEntityData(entity);
			std::vector<int>& components = data.Components[component.GetTypeId()];
			auto itr = std::find(components.begin(), components.end(), component.GetId());
			if (itr != components.end())
			{
				components.erase(itr);
				if (components.size() == 0)
				{
					data.Components.erase(component.GetTypeId());
				}
			}
		}

		ComponentHandle ECSEntityManager::GetComponent(Entity entity, ComponentTypeId componentType)
		{
			if (!IsValidEntity(entity) || !HasComponent(entity, componentType))
			{
				return { 0, -1 };
			}
			EntityData const& data = GetEntityData(entity);
			return { componentType, data.Components.find(componentType)->second.at(0) };
		}

		bool ECSEntityManager::IsValidEntity(Entity entity) const
		{
			if (entity < 0 
				&& entity >= m_entities.size()
				&& m_freeEntities.find(entity) != m_freeEntities.end())
			{
				return false;
			}
			return true;
		}

		int ECSEntityManager::GetComponentCount(Entity entity) const
		{
			if (!IsValidEntity(entity))
			{
				return 0;
			}
			return static_cast<int>(GetEntityData(entity).Components.size());
		}

		bool ECSEntityManager::HasComponent(Entity entity, ComponentTypeId componentType) const
		{
			if (!IsValidEntity(entity))
			{
				return false;
			}
			EntityData const& data = GetEntityData(entity);
			return data.Components.find(componentType) != data.Components.end() && data.Components.size() > 0;
		}

		EntityData& ECSEntityManager::GetEntityData(Entity entity)
		{
			ASSERT(entity >= 0 && entity < m_entities.size() && m_freeEntities.find(entity) == m_freeEntities.end());
			return m_entities.at(entity);
		}

		EntityData const& ECSEntityManager::GetEntityData(Entity entity) const
		{
			ASSERT(entity >= 0 && entity < m_entities.size() && m_freeEntities.find(entity) == m_freeEntities.end());
			return m_entities.at(entity);
		}
	}
}
#endif ///#ifdef IS_EXP_ECS