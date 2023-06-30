#pragma once

#include "Core/TypeAlias.h"
#include "Platforms/Platform.h"
#include "Runtime/Defines.h"
#include "Core/GUID.h"
#include "Algorithm/Vector.h"

#include "Core/Memory.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

#include "Generated/Entity_reflect_generated.h"

#include <Reflect.h>

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

	namespace Editor
	{
		class ComponentsOperation;
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

		REFLECT_CLASS()
		class IS_RUNTIME Component : public Serialisation::ISerialisable
		{
			REFLECT_GENERATED_BODY()

		public:
			Component();
			virtual ~Component();

			/*
			*	Each component should have a type name and override GetTypeName.
				static constexpr char* Type_Name = "ComponentClass";
				virtual const char* GetTypeName() override { return Type_Name; }
			*/
			static constexpr char* Type_Name = "Component";
			/// @brief  Return the component's type name.
			virtual const char* GetTypeName() { return "Component"; };

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

			Core::GUID GetGuid() const { return m_guid; }

			Entity* GetOwnerEntity() const { return m_ownerEntity; }
			bool IsEnabled() const { return m_isEnabled; }
			void SetEnabled(bool enabled) { m_isEnabled = enabled; }

			IS_SERIALISABLE_H(Component)

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
			// TODO Must add type information.
			/// @brief Store the unique ID for the component.
			Core::GUID m_guid;
			Entity* m_ownerEntity = nullptr;
			bool m_isEnabled = true;

			friend class Entity;
			friend class EntityManager;
			friend class Editor::ComponentsOperation;
		};

#define IS_COMPONENT(Component) \
		static constexpr const char* Type_Name = #Component; \
		virtual const char* GetTypeName() override { return Type_Name; }

		using ComponentRegistryMap = std::unordered_map<std::string, std::function<Component* ()>>;
		class IS_RUNTIME ComponentRegistry
		{
		public:
			static void RegisterComponent(std::string_view component_type, std::function<Component* ()> func);
			static void UnregisterComponent(std::string_view component_type);
			static Component* CreateComponent(std::string_view component_type);

			static std::vector<std::string> GetComponentNames();

		private:
			static ComponentRegistryMap m_register_funcs;
		};

		class IS_RUNTIME Entity : public Serialisation::ISerialisable
		{
		public:
#ifdef ECS_ENABLED
			Entity(ECSWorld* ecs_world);
			Entity(ECSWorld* ecs_world, std::string name);
#else
			Entity() = default;
			Entity(EntityManager* entity_manager);
			Entity(EntityManager* entity_manager, std::string name);
			~Entity();
#endif
			Entity* GetParent() const { return m_parent.Get(); }

			Ptr<Entity> AddChild();
			Ptr<Entity> AddChild(std::string entity_name);

			void RemoveChild(u32 index);

			u32 GetChildCount() const;
			Ptr<Entity> GetFirstChild() const;
			Ptr<Entity> GetLastChild() const;
			Ptr<Entity> GetChild(u32 index) const;

			Component* AddComponentByName(std::string_view component_type);

			void RemoveComponent(const Component* component);
			void RemoveComponent(const Core::GUID& guid);
			void RemoveComponent(std::string_view component_type);

			bool HasComponentByName(std::string_view component_type) const;
			template<typename T>
			bool HasComponent() const
			{
				for (RPtr<Component> const& component : m_components)
				{
					if (component->GetTypeName() == T::Type_Name)
					{
						return true;
					}
				}
				return false;
			}

			Component* GetComponentByGuid(const Core::GUID& guid) const;
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

			Component* GetComponentByIndex(u32 idx) const;
			u32 GetComponentCount() const;

			Core::GUID GetGUID() const { return m_guid; }

			std::string GetName() const { return m_name; }
			void SetName(std::string newName) { m_name = std::move(newName); }

			bool IsEnabled() const;
			void SetEnabled(bool enabled) { m_isEnabled = enabled; }

			IS_SERIALISABLE_H(Entity)

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
			bool m_isEnabled = true;

			std::vector<Ptr<Entity>> m_children;
			// TODO: Currently the Entity owns its components. Maybe a component manager should own all components
			// TODO: and the entity should just hold a view/pointer to it. (Need to think about lifetime, RPtr?)
			std::vector<RPtr<Component>> m_components;

			friend class EntityManager;
		};
#endif
	}

	namespace Serialisation
	{
		struct IS_RUNTIME EntityToGuid {};
		template<>
		struct IS_RUNTIME PropertySerialiser<EntityToGuid>
		{
			using InType = ECS::Entity;
			std::string operator()(Ptr<InType> const& v) const;
			std::string operator()(InType const& v) const;
		};
		template<>
		struct IS_RUNTIME PropertyDeserialiser<EntityToGuid>
		{
			using InType = std::string;
			ECS::Entity* operator()(InType const& v) const;
		};

		struct IS_RUNTIME ComponentToGuid {};
		template<>
		struct IS_RUNTIME PropertySerialiser<ComponentToGuid>
		{
			using InType = ECS::Component;
			std::string operator()(Ptr<InType> const& v) const;
			std::string operator()(InType const& v) const;
		};
		template<>
		struct IS_RUNTIME PropertyDeserialiser<ComponentToGuid>
		{
			using InType = std::string;
			ECS::Component* operator()(InType const& v) const;
		};

		struct EntityComponent1 {};
		template<>
		struct ComplexSerialiser<EntityComponent1, std::vector<RPtr<ECS::Component>>, ECS::Entity>
		{
			void operator()(ISerialiser* serialiser, std::vector<RPtr<ECS::Component>>& components, ECS::Entity* entity) const
			{
				if (serialiser->IsReadMode())
				{
					u64 componentArraySize = 0;
					serialiser->StartArray("Components", componentArraySize);
					for (auto const& c : components)
					{
						c->Deserialise(serialiser);
					}
					serialiser->StopArray();
				}
				else
				{
					u64 componentArraySize = components.size();
					serialiser->StartArray("Components", componentArraySize);
					for (auto const& c : components)
					{
						c->Serialise(serialiser);
					}
					serialiser->StopArray();
				}
			}
		};

		struct ComponentProperties1 {};
		template<>
		struct ComplexSerialiser<ComponentProperties1, void, ECS::Component>
		{
			void operator()(ISerialiser* serialiser, ECS::Component* component) const
			{
				Reflect::TypeInfo typeInfo = component->GetTypeInfo();
				std::vector<Reflect::MemberInfo> members = typeInfo.GetMemberInfosWithFlag("EditorVisible");
				Algorithm::VectorRemoveAllIf(members, [](const Reflect::MemberInfo& member)
				{
					return member.GetValueType() != Reflect::EReflectValueType::Value;
				});

				if (serialiser->IsReadMode())
				{
					u64 dynamicPropertiesSize = 0;
					serialiser->StartArray("DynamicProperties", dynamicPropertiesSize);
					for (int i = 0; i < dynamicPropertiesSize; ++i)
					{
						std::string propertyName;
						serialiser->Read("Name", propertyName);

						std::vector<Byte> data;
						serialiser->Read("Data", data);

						auto memberIter = Algorithm::VectorFindIf(members, [&propertyName](const Reflect::MemberInfo& member)
							{
								return member.GetMemberName() == propertyName;
							});

						if (memberIter != members.end())
						{
							Reflect::MemberInfo& member = (*memberIter);
							assert(member.GetType().GetTypeSize() == data.size());
							Platform::MemCopy(member.GetMemberPointer(), data.data(), data.size());
						}
					}
					serialiser->StopArray();
				}
				else
				{
					u64 dynamicPropertiesSize = members.size();
					serialiser->StartArray("DynamicProperties", dynamicPropertiesSize);
					for (const Reflect::MemberInfo& member : members)
					{
						serialiser->Write("Name", std::string(member.GetMemberName()));

						std::vector<Byte> memberData;
						memberData.resize(member.GetType().GetTypeSize());
						Platform::MemCopy(memberData.data(), member.GetMemberPointer(), memberData.size());

						serialiser->Write("Data", memberData);
					}
					serialiser->StopArray();
				}
			}
		};
	}

	OBJECT_SERIALISER(ECS::Component, 3,
		SERIALISE_PROPERTY(Core::GUID, m_guid, 1, 0)
		SERIALISE_PROPERTY(Serialisation::EntityToGuid, m_ownerEntity, 1, 0)
		SERIALISE_PROPERTY(bool, m_isEnabled, 1, 0)
		SERIALISE_COMPLEX_THIS(Serialisation::ComponentProperties1, 3, 0)
	);

	OBJECT_SERIALISER(ECS::Entity, 3,
		SERIALISE_PROPERTY(Core::GUID, m_guid, 1, 0)
		SERIALISE_PROPERTY(std::string, m_name, 1, 0)
		SERIALISE_PROPERTY(Serialisation::EntityToGuid, m_parent, 1, 0)
		SERIALISE_PROPERTY(bool, m_isEnabled, 1, 0)
		SERIALISE_VECTOR_PROPERTY(Serialisation::EntityToGuid, m_children, 1, 0)
		SERIALISE_COMPLEX(Serialisation::EntityComponent1, m_components, 3, 0)
	);
}