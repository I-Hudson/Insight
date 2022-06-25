#pragma once

#include "Core/TypeAlias.h"
#include "Core/Memory.h"

#include <unordered_map>
#include <typeinfo>

namespace Insight
{
	namespace App
	{
#ifdef IS_EXP_ECS

		template<typename ComponentType>
		u64 ComponentTypeToId()
		{
			return typeid(ComponentType).hash_code();
		}

		using ComponentHandle = Handle;

		class IECSComponentArray
		{
		public:
			virtual void Init() = 0;
			virtual void Free() = 0;

			virtual ComponentHandle AddComponent() = 0;
			virtual void RemoveComponent() = 0;
			virtual void Update(float deltaTime) = 0;
		};

		template<typename ComponentType>
		class ECSComponentArray : public IECSComponentArray
		{
			static_assert(std::is_pod_v<ComponentType>, "[ECSComponentArray] 'ComponentType' must be plain of data.");
		public:
			virtual void Init() override { }
			virtual void Free() override { }

			virtual ComponentHandle AddComponent() override { return 0; }
			virtual void RemoveComponent() override { }
			virtual void Update(float deltaTime) override { }

			// Dynamic array of POD component data.
			std::vector<ComponentType> m_componentData;
		};

		class IECSSystem
		{
		public:
			virtual void Update(IECSComponentArray* componentArray, float deltaTime) = 0;
		};

		template<typename ComponentType>
		class ECSSystem : public IECSSystem
		{
		};

		class ECS
		{
		public:
			ECS();

			void Update(float deltaTime);

			template<typename ComponentType>
			void RegisterSystem()
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

			template<typename ComponentType>
			ComponentHandle AddComponent()
			{
				IECSComponentArray* componentArray = GetComponentArray<ComponentType>();
				return componentArray->AddComponent();
			}

		private:
			template<typename ComponentType>
			IECSComponentArray* GetComponentArray()
			{
				const u64 componentTypeId = ComponentTypeToId<ComponentType>();
				if (m_componentArrays.find(componentTypeId) == m_componentArrays.end())
				{
					UPtr<IECSComponentArray> componentArray = MakeUPtr<ECSComponentArray<ComponentType>>();
					m_componentArrays[componentTypeId] = std::move(componentArray);
				}
				auto itr = m_componentArrays.find(componentTypeId);
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
			std::vector<u64> m_componentSystemsToUpdate;
			std::unordered_map<u64, UPtr<IECSComponentArray>> m_componentArrays;
			std::unordered_map<u64, UPtr<IECSSystem>> m_componentSystems;
		};

#else 
		class ECS
		{
		public:
			void Update(float deltaTime) { }
		};
	}
}

#endif //#ifdef IS_EXP_ECS