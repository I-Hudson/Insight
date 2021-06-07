#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Serialization/Serializable.h"
#include "Engine/Core/Maths/Frustum.h"

#include "Reflect.h"
#include "Generated/Component_reflect_generated.h"

class Entity;
class EntityManager;
class ComponentManager;

namespace Insight::Graphics
{
	struct RenderListView;
}

REFLECT_STRUCT()
struct ComponentData : REFLECT_BASE()
{
	REFLECT_GENERATED_BODY()

protected:
	REFLECT_PROPERTY(Editor)
	bool AllowRemovable = true;
};

class Component : REFLECT_BASE(), public Object
{
public:
	Component();
	Component(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~Component();

	virtual void OnUpdate(const float& a_deltaTime) { }
	virtual void OnDraw(Insight::Graphics::RenderListView* renderList, const glm::mat4& worldTransform, const Insight::Maths::Frustum& cameraFrustum) { }
	bool IsValid() { return m_entityID != -1; }
	Entity GetEntity();

	ComponentID GetComponentID() const { return m_componentID; }
	ComponentType GetComponentType() const { return m_componentType; }

	template<typename T>
	T& GetComponentData()
	{
		return m_componentManager->GetComponentData<T>(m_componentType, m_componentID);
	}

	ComponentData& GetComponentData();

protected:
	ComponentID m_componentID;
	ComponentType m_componentType;
	ComponentManager* m_componentManager;
	EntityID m_entityID;
	EntityManager* m_entityManager;

	friend ComponentManager;
};
