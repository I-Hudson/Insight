#pragma once

#include "Engine/Component/Component.h"

class PlayerController : public Component
{
public:
	PlayerController();
	explicit PlayerController(EntityManager* entityManager, const EntityID& entity);

	//virtual void OnCreate() override;
	virtual void OnUpdate(const float& deltaTime) override;

	//virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	//virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:
	glm::mat2 mat2;
	glm::mat3 mat3;

	REGISTER_DEC_TYPE(PlayerController);
};