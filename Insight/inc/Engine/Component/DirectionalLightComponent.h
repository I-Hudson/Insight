#pragma once

#include "Engine/Component/Component.h"
#include "Reflect.h"
#include "Generated/DirectionalLightComponent_reflect_generated.h"

#include <../vendor/glm/glm/glm.hpp>

enum class LightType
{
	Directional,
	Point,
	Spot
};

REFLECT_STRUCT()
struct DirectionalLightComponentData : public ComponentData
{
	REFLECT_GENERATED_BODY()

	REFLECT_PROPERTY(ShowInEditor)
	float FOV = 45.0f;
	REFLECT_PROPERTY(ShowInEditor)
	glm::vec3 Direction;

	REFLECT_PROPERTY(ShowInEditor)
	float NearPlane = 0.1f;
	REFLECT_PROPERTY(ShowInEditor)
	float FarPlane = 128.0f;
};

class DirectionalLightComponent : public Component
{
public:
	DirectionalLightComponent() { }
	DirectionalLightComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~DirectionalLightComponent() override;

	virtual void OnUpdate(const float& a_deltaTime) override;
	//virtual void OnCreate() override;
	//virtual void OnDestroy() override;

	//virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	//virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:
	REGISTER_DEC_TYPE(DirectionalLightComponent);
};