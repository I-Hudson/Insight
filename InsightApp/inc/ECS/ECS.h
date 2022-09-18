#pragma once

#include "Core/TypeAlias.h"
#include "Core/Memory.h"

#include <unordered_map>
#include <queue>
#include <unordered_set>
#include <typeinfo>
#include <functional>

///#define IS_EXP_ECS

namespace Insight
{
	namespace App
	{
#ifdef IS_EXP_ECS

		template<typename>
		class ECSComponentArray;
		class ECS;

		using Entity = int;

		using ComponentTypeId = u64;
		template<typename ComponentType>
		ComponentTypeId ComponentTypeToId()
		{
			return typeid(ComponentType).hash_code();
		}

		struct ComponentHandle
		{
			ComponentHandle() { }
			ComponentHandle(ComponentTypeId typeId, int slotIndex) : m_componentType(typeId), m_slotIndex(slotIndex) { }

			ComponentTypeId GetTypeId() const { return m_componentType; }
			int GetId() const { return m_slotIndex; }

		private: 
			ComponentTypeId m_componentType = -1;
			int m_slotIndex = 0;
			template<typename>
			friend class ECSComponentArray;
		};

		class IECSComponentArray
		{
		protected:
			const static u64 COMPONENT_ARRAY_START_SIZE = 128;
		public:
			virtual void Init() = 0;
			virtual void Free() = 0;

			virtual ComponentHandle AddComponent(Entity entity) = 0;
			virtual void RemoveComponent(ComponentHandle handle) = 0;
			virtual void Update(float deltaTime) = 0;

			virtual Entity GetOwnerEntity(ComponentHandle componentHandle) = 0;
			template<typename ComponentType>
			ComponentType& GetComponent(ComponentHandle handle)
			{
				return static_cast<ComponentType&>(*GetComponent(handle));
			}
			template<typename ComponentType>
			ComponentType GetComponent(Entity entity)
			{
				return *static_cast<ComponentType*>(GetComponent(entity));
			}

		private:
			virtual void* GetComponent(ComponentHandle handle) = 0;
			virtual void* GetComponent(Entity entity) = 0;
		};

		template<typename ComponentType>
		class ECSComponentArray : public IECSComponentArray
		{
			struct ComponentData
			{
				ComponentType ComponentType;
				Entity Entity;
			};
			///static_assert(std::is_pod_v<ComponentType>, "[ECSComponentArray] 'ComponentType' must be plain of data.");
		public:
			virtual void Init() override 
			{ 
				m_componentData.reserve(COMPONENT_ARRAY_START_SIZE);
			}
			virtual void Free() override 
			{
				m_componentData.clear();
				while (m_freeSlots.size() > 0)
				{
					m_freeSlots.pop();
				}
			}

			virtual ComponentHandle AddComponent(Entity entity) override
			{
				if (m_freeSlots.size() == 0)
				{
					m_componentData.push_back(ComponentData());
					m_freeSlots.push((int)m_componentData.size() - 1);
				}

				int slotIndex = m_freeSlots.front();
				m_freeSlots.pop();

				ComponentHandle handle;
				handle.m_componentType = ComponentTypeToId<ComponentType>();
				handle.m_slotIndex = slotIndex;

				m_componentData[slotIndex].Entity = entity;
				m_componentToEntity[handle.m_slotIndex] = entity;
				return handle;
			}
			virtual void RemoveComponent(ComponentHandle handle) override 
			{ 
				m_componentData[handle.GetId()] = {};
				m_freeSlots.push(handle.GetId());
				m_componentToEntity[handle.GetId()] = -1;
			}

			virtual void Update(float deltaTime) override 
			{

			}

			virtual Entity GetOwnerEntity(ComponentHandle componentHandle) override
			{
				return m_componentData.at(componentHandle.GetId()).Entity;
			}

		private:
			virtual void* GetComponent(ComponentHandle handle) override
			{
				ASSERT(handle.GetId() > 0 && handle.GetId() < m_componentData.size());
				return &m_componentData.at(handle.GetId());
			}
			virtual void* GetComponent(Entity entity) override
			{
				auto itr = m_componentToEntity.find(entity);
				ASSERT(itr != m_componentToEntity.end());
				return &m_componentData.at(itr->second);
			}

		public:
			/// Dynamic array of POD component data.
			std::vector<ComponentData> m_componentData;
			std::unordered_map<int, int> m_componentToEntity;
			std::queue<int> m_freeSlots;
		};

		class IECSSystem
		{
		protected:
			virtual void Update(ECS& ecs, IECSComponentArray* componentArray, float deltaTime) = 0;
			friend class ECS;
		};

		template<typename ComponentType>
		class ECSSystemImpl
		{
		public:
			static void Update(ECS& ecs, Entity ownerEnttity, ComponentType& component, float deltaTime)
			{ }
		};

		template<typename ComponentType>
		class ECSSystem : public IECSSystem
		{
		protected:
			virtual void Update(ECS& ecs, IECSComponentArray* componentArray, float deltaTime) override final
			{
				ECSComponentArray<ComponentType>* compoenentArray = static_cast<ECSComponentArray<ComponentType>*>(componentArray);
				for (size_t i = 0; i < compoenentArray->m_componentData.size(); ++i)
				{
					ComponentType& component = compoenentArray->m_componentData[i].ComponentType;
					ECSSystemImpl<ComponentType>::Update(ecs
						, compoenentArray->m_componentData[i].Entity
						, component
						, deltaTime);
				}
			}
		};

		struct EntityData
		{
			std::unordered_map<ComponentTypeId, std::vector<int>> Components;
		};
		class ECSEntityManager
		{
		public:

			Entity AddEntity();
			void RemoveEntity(Entity& entity);

			void AddComponentToEntity(Entity entity, ComponentHandle component);
			void RemoveComponentFromEntity(Entity entity, ComponentHandle component);
			ComponentHandle GetComponent(Entity entity, ComponentTypeId componentType);

			bool IsValidEntity(Entity entity) const;
			int GetComponentCount(Entity entity) const;
			bool HasComponent(Entity entity, ComponentTypeId componentType) const;

		private:
			EntityData& GetEntityData(Entity entity);
			EntityData const& GetEntityData(Entity entity) const;

		private:
			std::vector<EntityData> m_entities;
			std::unordered_set<int> m_freeEntities;
		};

		struct ECSRequests
		{
			using ECSRequestFunction = std::function<void()>;
			std::vector<ECSRequestFunction> AddRequests;
			std::vector<ECSRequestFunction> RemoveRequests;
		};

		//// <summary>
		//// Store a copy of the previous frames data so the ECS can be threaded.
		//// </summary>
		class ECSFrameUpdateData
		{
		private:
			std::unordered_map<ComponentTypeId, UPtr<IECSComponentArray>> m_componentArrays;
			friend class ECS;
		};
		class ECS
		{
		public:
			ECS();

			void Update(float deltaTime);

			template<typename Component>
			void RegisterComponent()
			{
				const u64 componentTypeId = ComponentTypeToId<ComponentType>();
				GetComponentArray<ComponentType>();
				GetComponentSystem<ComponentType>();
				auto itr = std::find(m_componentSystemsToUpdate.begin(), m_componentSystemsToUpdate.end(), componentTypeId);
				if (itr == m_componentSystemsToUpdate.end())
				{
					m_componentSystemsToUpdate.push_back(componentTypeId);
				}
			}

			Entity AddEntity() { return m_entityManager.AddEntity(); }
			void RemoveEntity(Entity& entity) { m_entityManager.RemoveEntity(entity); }

			Entity GetOwnerEntity(ComponentHandle componentHandle);

			template<typename ComponentType>
			void AddComponent(Entity entity)
			{
				if (!m_entityManager.IsValidEntity(entity))
				{
					return;
				}
			
				IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				ComponentHandle handle = componentArray->AddComponent(entity);
				m_entityManager.AddComponentToEntity(entity, handle);
				///m_requests.AddRequests.push_back([this, entity]()
				///	{
				///		IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				///		ComponentHandle handle = componentArray->AddComponent();
				///		m_entityManager.AddComponentToEntity(entity, handle);
				///	});
			}
			template<typename ComponentType>
			void RemoveComponent(Entity entity)
			{
				const u64 componentTypeId = ComponentTypeToId<ComponentType>();
				if (!m_entityManager.IsValidEntity(entity) || !m_entityManager.HasComponent(entity, componentTypeId))
				{
					return;
				}
				ComponentHandle handle = m_entityManager.GetComponent(entity, componentTypeId);
				m_entityManager.RemoveComponentFromEntity(entity, handle);

				IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				componentArray->RemoveComponent(handle);

				///m_requests.RemoveRequests.push_back([this, entity, componentTypeId]()
				///	{
				///		ComponentHandle handle = m_entityManager.GetComponent(entity, componentTypeId);
				///		m_entityManager.RemoveComponentFromEntity(entity, handle);
				///
				///		IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				///		componentArray->RemoveComponent(handle);
				///	});
			}

			template<typename ComponentType>
			ComponentType GetComponent(Entity entity)
			{
				if (!m_entityManager.IsValidEntity(entity))
				{
					return ComponentType();
				}

				IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				return componentArray->GetComponent<ComponentType>(entity);
			}

		private:
			void UpdateRequests();
			void PrepareFrameUpdateData();
			
			template<typename ComponentType>
			IECSComponentArray* GetComponentArray()
			{
				const u64 componentTypeId = ComponentTypeToId<ComponentType>();
				if (m_componentArrays.find(componentTypeId) == m_componentArrays.end())
				{
					UPtr<IECSComponentArray> componentArray = MakeUPtr<ECSComponentArray<ComponentType>>();
					m_componentArrays[componentTypeId] = std::move(componentArray);
					m_componentArrays[componentTypeId]->Init();
				}
				auto itr = m_componentArrays.find(componentTypeId);
				ASSERT(itr != m_componentArrays.end());
				return itr->second.Get();
			}
			IECSComponentArray* GetComponentArray(ComponentHandle componentHandle)
			{
				auto itr = m_componentArrays.find(componentHandle.GetTypeId());
				ASSERT(itr != m_componentArrays.end());
				return itr->second.Get();
			}

			template<typename ComponentType>
			IECSSystem* GetComponentSystem()
			{
				const u64 componentTypeId = ComponentTypeToId<ComponentType>();
				if (m_componentSystems.find(componentTypeId) == m_componentSystems.end())
				{
					UPtr<IECSSystem> componentSystem = MakeUPtr<ECSSystem<ComponentType>>();
					m_componentSystems[componentTypeId] = std::move(componentSystem);
				}
				auto itr = m_componentSystems.find(componentTypeId);
				ASSERT(itr != m_componentSystems.end());
				return itr->second.Get();
			}

		private:
			ECSRequests m_requests;
			ECSFrameUpdateData m_frameUpdateData;
			ECSEntityManager m_entityManager;

			std::vector<ComponentTypeId> m_componentSystemsToUpdate;
			std::unordered_map<ComponentTypeId, UPtr<IECSComponentArray>> m_componentArrays;
			std::unordered_map<ComponentTypeId, UPtr<IECSSystem>> m_componentSystems;
		};

#else 
		class ECS
		{
		public:
			void Update(float deltaTime) { }
		};
#endif ///#ifdef IS_EXP_ECS
	}
}
