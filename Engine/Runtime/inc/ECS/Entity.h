#pragma once

#include "Core/TypeAlias.h"
#include "Platforms/Platform.h"
#include "Runtime/Defines.h"
#include "Core/GUID.h"

#include "Core/Memory.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#ifdef IS_ECS_ENABLED
#include <unordered_set>
#endif
#include <functional>

namespace Insight
{
	namespace App
	{
		class Scene;
	}

	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		class EntityManager;
		class ECSWorld;

		//// <summary>
		//// Store all relevant data for an entity.
		//// This is not stored on the Entity as the 
		//// </summary>
		struct IS_RUNTIME EntityData
		{
			std::string Name;
			Core::GUID GUID = Core::GUID::s_InvalidGUID;
			std::unordered_map<u64, std::unordered_set<int>> Components;
		};

		//// <summary>
		//// Entity is a simple handle for an entity. This class
		//// shouldn't hold/store any critical data.
		//// </summary>
		class IS_RUNTIME Entity
		{
		public:

			void SetName(std::string name);

			std::string GetName() const;
			int GetId() const;
			Core::GUID GetGuid() const;

			bool IsVaild() const;

		private:
			int m_id = -1;
			EntityManager* m_entityManager = nullptr;
			ECSWorld* m_ecsWorld = nullptr;
			friend class EntityManager;
		};

		const Entity c_InvalidEntity = Entity();
#else
		class ECSWorld;
		class EntityManager;
		class Entity;

		class IS_RUNTIME Component
		{
		public:
			Component();
			virtual ~Component();

			/*
			*	Each component should have a type name and override GetTypeName.
				static constexpr char* Type_Name = "ComponentClass";
				virtual const char* GetTypeName() override { return Type_Name; }
			*/

			/// @brief Called on creation (When the object is newed).
			virtual void OnCreate() { }
			/// @brief Called on destruction (Just before the object is deleted).
			virtual void OnDestroy() { }

			/// @brief  Called before first update (Only called once. Is called after Update but before Early Update).
			virtual void OnBegin() { }
			/// @brief  Called after last update (Only called once, Is called after Update but before Late Update).
			virtual void OnEnd() { }

			/// @brief  Called before any update.
			virtual void OnEarlyUpdate() { }
			/// @brief  Called once per-frame 
			virtual void OnUpdate(const float delta_time) { }
			/// @brief  Called after every udpate.
			virtual void OnLateUpdate() { }
			/// @brief  Return the component's type name.
			virtual const char* GetTypeName() = 0;

			Entity* GetOwnerEntity() const { return m_ownerEntity; }

		protected:
			/// @brief  Allow multiple of the same component to be added to a single entity. Default is true
			bool m_allow_multiple : 1;
			/// @brief  Allow the component to be removed from an entity. Default is true.
			bool m_removeable : 1;

			/// @brief Track when OnBegin has been called (Should only be called once, for each time this object is Enabled).
			bool m_on_begin_called : 1;
			/// @brief Track when OnEnd has been called (Should only be called once, for each time this object is Enabled).
			bool m_on_end_called : 1;

		private:
			/// @brief Store the unique ID for the component.
			Core::GUID m_guid;
			Entity* m_ownerEntity = nullptr;
			// TODO Must add type information.

			friend class Entity;
		};
#define IS_COMPONENT(Component) \
		static constexpr char* Type_Name = #Component; \
		virtual const char* GetTypeName() override { return Type_Name; }

		using ComponentRegistryMap = std::unordered_map<std::string, std::function<Component*()>>;
		class ComponentRegistry
		{
		public:
			static void RegisterComponent(std::string_view component_type, std::function<Component*()> func);
			static Component* CreateComponent(std::string_view component_type);

			static void RegisterInternalComponents();

		private:
			static ComponentRegistryMap m_register_funcs;
		};

		class IS_RUNTIME Entity
		{
		public:
#ifdef ECS_ENABLED
			Entity(ECSWorld* ecs_world);
			Entity(ECSWorld* ecs_world, std::string name);
#else
			Entity(EntityManager* entity_manager);
			Entity(EntityManager* entity_manager, std::string name);
#endif

			Ptr<Entity> AddChild();
			Ptr<Entity> AddChild(std::string entity_name);

			void RemoveChild(u32 index);

			u32 GetChildCount() const;
			Ptr<Entity> GetFirstChild() const;
			Ptr<Entity> GetLastChild() const;
			Ptr<Entity> GetChild(u32 index) const;

			Component* AddComponentByName(std::string_view component_type);
			void RemoveComponentByName(std::string_view component_type);

			bool HasComponentByName(std::string_view component_type) const;
			template<typename T>
			bool HasComponent() const
			{
				return GetComponent<T>() != nullptr;
			}

			Component* GetComponentByName(std::string_view component_type) const;
			template<typename T>
			T* GetComponentByName(std::string_view component_type) const
			{
				static_assert(std::is_base_of_v<Component, T>);
				Component* component = GetComponentByName(component_type);
				if (component)
				{
					return static_cast<T*>(component);
				}
				return nullptr;
			}

			template<typename T>
			T* GetComponent() const
			{
				static_assert(std::is_base_of_v<Component, T>);
				Component* component = GetComponentByName(T::Type_Name);
				if (component)
				{
					return static_cast<T*>(component);
				}
				return nullptr;
			}

			Core::GUID GetGUID() const { return m_guid; }

			std::string GetName() const { return m_name; }
			void SetName(std::string entity_name) { m_name = std::move(m_name); }

		private:
			void EarlyUpdate();
			void Update(const float delta_time);
			void LateUpdate();

			void Destroy();

		private:
			Core::GUID m_guid;
			std::string m_name;

			Ptr<Entity> m_parent = nullptr;
#ifdef ECS_ENABLED
			ECSWorld* m_ecs_world = nullptr;
#else
			EntityManager* m_entity_manager = nullptr;
#endif

			std::vector<Ptr<Entity>> m_children;
			// TODO: Currently the Entity owns its components. Maybe a component manager should own all components
			// TODO: and the entity should just hold a view/pointer to it. (Need to think about lifetime, RPtr?)
			std::vector<RPtr<Component>> m_components;

			friend class EntityManager;
		};

#endif
	}
}