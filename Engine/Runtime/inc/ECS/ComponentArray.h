#pragma once

#ifdef IS_ECS_ENABLED
#include "Core/TypeAlias.h"
#include "ECS/ComponentHandle.h"
#include "ECS/Entity.h"
#include "ECS/ECSSystenFuncs.h"
#include "Core/Memory.h"

#include <vector>
#include <unordered_map>
#include <queue>
#include <shared_mutex>
#endif

#include "Core/TypeAlias.h"
#include "Core/Logger.h"
#include "Algorithm/Vector.h"

#include "ECS/Entity.h"

#include <Reflect/Reflect.h>

#include <vector>
#include <set>

namespace Insight
{
	namespace ECS
	{
		class Component;
		using ComponentId = i32;

		class IComponentArray
		{
		public:
			IComponentArray(Reflect::TypeInfo typeInfo);
			virtual ~IComponentArray();

			virtual void OnEarlyUpdate() = 0;
			virtual void OnUpdate(const float deltaTime) = 0;
			virtual void OnLateUpdate() = 0;

			virtual ComponentId AddComponent() = 0;
			virtual bool RemoveComponent(const ComponentId componentId) = 0;

			template<typename T>
			T* GetComponent(const ComponentId componentId)
			{
				Component* component = GetComponentPtr(componentId);
				if (!component)
				{
					return nullptr;
				}

				T* componentType = dynamic_cast<T*>(component);
				return componentType;
			}

			const Reflect::TypeInfo& GetTypeInfo() const;

		protected:
			virtual Component* GetComponentPtr(const ComponentId componentId) = 0;

		protected:
			Reflect::TypeInfo m_typeInfo;
		};

		template<typename T>
		class ComponentTypeArray : public IComponentArray
		{
			static_assert(std::is_base_of_v<Component, T>);

		public:
			ComponentTypeArray(Reflect::TypeInfo typeInfo)
				: IComponentArray(std::move(typeInfo))
			{ }

			virtual ~ComponentTypeArray() override
			{ }

			virtual void OnEarlyUpdate() override
			{
				for (size_t i = 0; i < m_components.size(); ++i)
				{
					Component* component = static_cast<Component*>(&m_components.at(i));
					component->OnEarlyUpdate();
				}
			}
			virtual void OnUpdate(const float deltaTime) override
			{
				for (size_t i = 0; i < m_components.size(); ++i)
				{
					Component* component = static_cast<Component*>(&m_components.at(i));
					component->OnUpdate(deltaTime);
				}
			}
			virtual void OnLateUpdate() override
			{
				for (size_t i = 0; i < m_components.size(); ++i)
				{
					Component* component = static_cast<Component*>(&m_components.at(i));
					component->OnLateUpdate();
				}
			}

			virtual ComponentId AddComponent() override
			{
				if (m_freeList.empty())
				{
					m_components.push_back(T());
					const i32 newFreeSlot = m_components.size() - 1;

					assert(m_freeList.find(newFreeSlot) == m_freeList.end());
					m_freeList.insert(newFreeSlot);
				}

				const i32 freeSlot = *m_freeList.begin();
				m_freeList.erase(freeSlot);
				return freeSlot;
			}

			virtual bool RemoveComponent(const ComponentId componentId) override
			{
				if (!Algorithm::VectorIndexWithinRange(m_components, componentId))
				{
					IS_LOG_CORE_WARN("[ComponentTypeArray::RemoveComponent] ComponentId '{}' is out of range. Enter a valid ComponentId.");
					return false;
				}

				if (m_freeList.find(componentId) != m_freeList.end())
				{
					IS_LOG_CORE_WARN("[ComponentTypeArray::RemoveComponent] ComponentId '{}' is already within the free list.");
					return false;
				}

				m_components.at(componentId) = T();
				m_freeList.insert(componentId);
			}

		protected:
			virtual Component* GetComponentPtr(const ComponentId componentId) override
			{
				if (!Algorithm::VectorIndexWithinRange(m_components, componentId))
				{
					return nullptr;
				}

				if (m_freeList.find(componentId) != m_freeList.end())
				{
					IS_LOG_CORE_WARN("[ComponentTypeArray::RemoveComponent] ComponentId '{}' is already within the free list.");
					return nullptr;
				}

				T& componentRef = m_components[componentId];
				return static_cast<Component*>(&componentRef);
			}

		private:
			std::vector<T> m_components;
			std::set<ComponentId> m_freeList;
		};

		class ComponentManager
		{
		public:
			ComponentManager();
			~ComponentManager();

			template<typename T>
			ComponentId AddComponent()
			{
				if (!ComponentRegistry::HasComponent(T::Type_Name))
				{
					return -1;
				}

				const Reflect::TypeInfo typeInfo = T::GetStaticTypeInfo();

				IComponentArray* foundComponentArray = nullptr;
				for (size_t i = 0; i < m_componentArrays.size(); ++i)
				{
					IComponentArray* componentArray = m_componentArrays.at(i);
					if (typeInfo == componentArray->GetTypeInfo())
					{
						foundComponentArray = componentArray;
						break;
					}
				}

				if (!foundComponentArray)
				{
					foundComponentArray = ::New<ComponentTypeArray<T>>(typeInfo);
					m_componentArrays.push_back(foundComponentArray);
				}

				return foundComponentArray->AddComponent();
			}

			template<typename T>
			T* GetComponent(const ComponentId componentId) const
			{
				static_assert(std::is_base_of_v<Component, T>);
				const Reflect::TypeInfo typeInfo = T::GetStaticTypeInfo();
				for (size_t i = 0; i < m_componentArrays.size(); ++i)
				{
					IComponentArray* componentArray = m_componentArrays.at(i);
					if (typeInfo == componentArray->GetTypeInfo())
					{
						return componentArray->GetComponent<T>(componentId);
					}
				}

				return nullptr;
			}

			void OnEarlyUpdate() const;
			void OnUpdate(const float deltaTime) const;
			void OnLateUpdate() const;

		private:
			std::vector<IComponentArray*> m_componentArrays;
		};

#ifdef IS_ECS_ENABLED
		/*
		//// <summary>
		//// This acts like string_view. This should only "look" into a component array
		//// but shouldn't take any form of ownership.
		//// </summary>
		//// <typeparam name="Component"></typeparam>
		template<typename Component>
		class ComponentArrayView
		{
		public:
			ComponentArrayView(const std::vector<Component>* const view, int startIndex, int endIndex)
				: m_componentsView(view)
				, m_startIndex(startIndex)
				, m_endIndex(endIndex)
			{ }
		
			Component& ForEach(std::is_function) 
			{
				const u64 size = m_componentsView->size();
				for (size_t i = 0; i < size; ++i)
				{
		
				}
			}
		
		private:
			const std::vector<Component>* const m_componentsView = nullptr;
			int m_startIndex = 0;
			int m_endIndex = 0;
		};
		*/

		class ECSWorld;

		//// <summary>
		//// Base class for a templated component array.
		//// </summary>
		class IComponentArray
		{
		public:
			IComponentArray(u64 componentTypeId)
				: m_componentTypeId(componentTypeId)
			{ }

			virtual ComponentHandle AddComponent(Entity entity) = 0;
			virtual void RemoveComponent(Entity entity, ComponentHandle& handle) = 0;
			virtual void Update(float deltaTime) = 0;

			template<typename Component>
			Component GetComponent(int componentIndex)
			{
				return *static_cast<Component*>(GetComponent(componentIndex));
			}

		protected:
			virtual void* GetComponent(int componentIndex) = 0;

		protected:
			const u64 m_componentTypeId = 0;
			std::queue<int> m_freeComponents;
		};

		//// <summary>
		//// Component array is a class which stores all components of a type.
		//// This should be used as mainly a store nothing else.
		//// You can add a component, remove, or get a ComponentArrayView.
		//// </summary>
		//// <typeparam name="Component"></typeparam>
		template<typename Component>
		class ComponentArray : public IComponentArray
		{
		public:
			ComponentArray(ECSWorld* ecsWorld, u64 componentTypeId)
				: m_ecsWorld(ecsWorld), IComponentArray(componentTypeId)
			{ }

			virtual ComponentHandle AddComponent(Entity entity) override
			{
				std::lock_guard lock(m_lock);
				if (m_freeComponents.size() == 0)
				{
					const int index = static_cast<int>(m_components.size());
					m_components.push_back(Component());
					m_freeComponents.push(index);
				}

				const int freeIndex = m_freeComponents.front();
				m_freeComponents.pop();

				ECSSystemOnAdded<Component>::OnAdded(
					ECSSystemOnAddedData(m_ecsWorld)
					, m_components.at(freeIndex));

				return ComponentHandle(entity.GetId(), freeIndex, m_componentTypeId);
			}
			virtual void RemoveComponent(Entity entity, ComponentHandle& handle) override
			{
				std::lock_guard lock(m_lock);
				m_freeComponents.push(handle.GetIndex());
				m_components.at(handle.GetIndex()) = { };
				handle = { };
			}
			virtual void Update(float deltaTime) override
			{
				const u64 componentCount = m_components.size();
				const ECSSystemOnUpdateData onUpdateData(m_ecsWorld, deltaTime);
				for (size_t i = 0; i < componentCount; ++i)
				{
					ECSSystemOnUpdate<Component>::OnUpdate(onUpdateData, m_components.at(i));
				}
			}

		protected:
			virtual void* GetComponent(int componentIndex) override
			{
				ASSERT(componentIndex >= 0 && componentIndex < static_cast<int>(m_components.size()));
				return &m_components.at(componentIndex);
			}

		private:
			ECSWorld* const m_ecsWorld = nullptr;
			std::vector<Component> m_components;
			std::shared_mutex m_lock;

			template<typename>
			friend class ComponentArray;
		};

		class ComponentArrayManager
		{
		public:
			ComponentArrayManager(ECSWorld* const ecsWorld);

			void Update(float deltaTime);

			template<typename Compoennt>
			ComponentHandle AddComponent(Entity entity)
			{
				const u64 componentType = GetComponentId<Compoennt>();
				IComponentArray* componentArray = GetComponentArray(componentType);
				if (componentArray == nullptr)
				{
					m_componentArrays[componentType] = MakeUPtr<ComponentArray<Compoennt>>(m_ecsWorld, componentType);
					componentArray = GetComponentArray(componentType);
				}
				return componentArray->AddComponent(entity);
			}

			void RemoveComponent(Entity entity, ComponentHandle& handle);

		private:
			IComponentArray* GetComponentArray(u64 componentType)
			{
				if (auto itr = m_componentArrays.find(componentType);
					itr != m_componentArrays.end())
				{
					return itr->second.Get();
				}
				return nullptr;
			}

		private:
			ECSWorld *const m_ecsWorld = nullptr;
			std::unordered_map<u64, UPtr<IComponentArray>> m_componentArrays;
			std::shared_mutex m_lock;
		};
#endif
	}
}