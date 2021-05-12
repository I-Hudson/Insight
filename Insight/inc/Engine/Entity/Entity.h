#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Component/Component.h"

class EntityManager;

struct EntityData
{
	std::string	Name;
	EntityComponentSignature Signature;
};

class IS_API Entity : public Object
					, public Serialization::Serializable
{
public:
	Entity();
	explicit Entity(EntityManager* entityManager, EntityID id);
	~Entity();

	void SetName(const std::string& name);

	bool IsValid() { return m_entityID != -1; }

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args);
	template<typename T>
	T& GetComponent();
	template<typename T>
	void RemoveComponent();
	template<typename T>
	bool HasComponent();

	virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

	void SetParent(const EntityID& entity);
	Entity GetParent();

	const EntityID& GetEntityID() const { return m_entityID; }
	const EntityID& GetParentEntityID() const { return m_parentEntityID; }

private:
	EntityID m_entityID;
	EntityID m_parentEntityID;
	EntityManager* m_entityManager;
	bool m_isValid;

	REGISTER_DEC_TYPE(Entity);
	friend EntityManager;
};

#include "Engine/Entity/EntityManager.h"

template<typename T, typename ...Args>
INLINE T& Entity::AddComponent(Args&&... args)
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::AddComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::AddComponent] Entity is not valid. Check 'IsValid' before this call.")
	return m_entityManager->AddComponent<T>(m_entityID, std::forward<Args...>(args)...);
}

template<typename T>
INLINE T& Entity::GetComponent()
{
	STATIC_ASSERT((std::is_base_of_v<Component, T>), "[Entity::GetComponent] template 'T' does not inherit from 'Component'");
	ASSERT(IsValid() && "[Entity::GetComponent] Entity is not valid. Check 'IsValid' before this call.")
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