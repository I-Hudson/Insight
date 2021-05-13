#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Component/Component.h"
#include "Reflect.h"
#include "Generated/Entity_reflect_generated.h"

class EntityManager;

REFLECT_STRUCT()
struct EntityData : REFLECT_BASE()
{
	REFLECT_GENERATED_BODY()
	std::string	Name;
	EntityComponentSignature Signature;
	std::vector<std::pair<ComponentType, ComponentID>> ComponentIDs;

	EntityID ParentEntityID = -1;
	std::vector<EntityID> EntityChildrenIDs;
	bool IsActive = false;

private:
	REFLECT_PROPERTY(EditorOnly)
	bool ShowDebug;
};

IS_API class Entity : public Object, public Serialization::Serializable
{
public:
	Entity();
	explicit Entity(EntityManager* entityManager, EntityID id);
	~Entity();

	void SetName(const std::string& name);
	std::string GetName();

	bool IsValid() { return m_entityID != -1; }
	u32 GetChildCount();
	Entity GetChild(const EntityID& entity);

	template<typename T>
	T& AddComponent();

	template<typename T>
	T& GetComponent();
	
	template<typename T>
	void RemoveComponent();
	void RemoveComponent(const Component& component);

	template<typename T>
	bool HasComponent();

	u32 GetComponentCount();
	Component& GetComponent(const u32& index);
	//std::vector<Component> GetAllComponents();

	virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

	void SetParent(const EntityID& entity);
	Entity GetParent();

	const EntityID& GetEntityID() const { return m_entityID; }
	const EntityID& GetParentEntityID() { return GetEntiyData().ParentEntityID; }

	EntityData& GetEntiyData();

private:
	EntityID m_entityID;
	EntityManager* m_entityManager;

	REGISTER_DEC_TYPE(Entity);
	friend EntityManager;
};

#include "Engine/Entity/EntityManager.h"

template<typename T>
INLINE T& Entity::AddComponent()
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::AddComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::AddComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->AddComponent<T>(m_entityID);
}

template<typename T>
INLINE T& Entity::GetComponent()
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::GetComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::GetComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->GetComponent<T>(m_entityID);
}

template<typename T>
INLINE void Entity::RemoveComponent()
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::RemoveComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::GetComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->RemoveComponent<T>(m_entityID);
}

template<typename T>
INLINE bool Entity::HasComponent()
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::RemoveComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::GetComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->HasComponent<T>(m_entityID);
}