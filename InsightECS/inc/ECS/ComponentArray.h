#pragma once

#include "Core/TypeAlias.h"
#include "ECS/ComponentHandle.h"
#include "ECS/Entity.h"
#include "Core/Memory.h"

#include <vector>
#include <unordered_map>
#include <queue>
#include <shared_mutex>

namespace Insight
{
	namespace ECS
	{
		/// <summary>
		/// This acts like string_view. This should only "look" into a component array
		/// but shouldn't take any form of ownership.
		/// </summary>
		/// <typeparam name="Component"></typeparam>
		//template<typename Component>
		//class ComponentArrayView
		//{
		//public:
		//	ComponentArrayView(const std::vector<Component>* const view, int startIndex, int endIndex)
		//		: m_componentsView(view)
		//		, m_startIndex(startIndex)
		//		, m_endIndex(endIndex)
		//	{ }
		//
		//	Component& ForEach(std::is_function) 
		//	{
		//		const u64 size = m_componentsView->size();
		//		for (size_t i = 0; i < size; ++i)
		//		{
		//
		//		}
		//	}
		//
		//private:
		//	const std::vector<Component>* const m_componentsView = nullptr;
		//	int m_startIndex = 0;
		//	int m_endIndex = 0;
		//};

		/// <summary>
		/// Base class for a templated component array.
		/// </summary>
		class IComponentArray
		{
		public:
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
		};

		/// <summary>
		/// Component array is a class which stores all components of a type.
		/// This should be used as mainly a store nothing else.
		/// You can add a component, remove, or get a ComponentArrayView.
		/// </summary>
		/// <typeparam name="Component"></typeparam>
		template<typename Component>
		class ComponentArray : public IComponentArray
		{
		public:
			virtual ComponentHandle AddComponent(Entity entity) override;
			virtual void RemoveComponent(Entity entity, ComponentHandle& handle) override;
			virtual void Update(float deltaTime) override;

		protected:
			virtual void* GetComponent(int componentIndex) override;

		private:
			std::vector<Component> m_components;

			template<typename>
			friend class ComponentArray;
		};

		class ComponentArrayManager
		{
		public:

			template<typename Compoennt>
			ComponentHandle AddComponent(Entity entity)
			{
				const u64 componentType = 0;
				IComponentArray* componentArray = GetComponentArray(componentType);
				if (componentArray == nullptr)
				{
					m_componentArrays[componentType] = MakeUPtr<ComponentArray<Compoennt>>();
					componentArray = GetComponentArray(componentType);
				}
				return componentArray->AddComponent(entity);
			}

			void RemoveComponent(Entity entity, ComponentHandle& handle);

		private:
			IComponentArray* GetComponentArray(u64 componentType)
			{
				return nullptr;
			}

		private:
			std::unordered_map<u64, UPtr<IComponentArray>> m_componentArrays;
			std::shared_mutex m_lock;
		};
	}
}