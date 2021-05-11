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
T& Entity::AddComponent(Args&&... args)
{
	ASSERT(IsValid() && "[Entity::AddComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->AddComponent<T>(m_entityID, std::forward<Args...>(args)...);
}

template<typename T>
	inline T& Entity::GetComponent()
{
	ASSERT(IsValid() && "[Entity::GetComponent] Entity is not valid. Check 'IsValid' before this call.");
	return m_entityManager->GetComponent<T>(m_entityID);
}