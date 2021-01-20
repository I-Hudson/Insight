#pragma once

#include "Insight/Component/Component.h"

class PlayerController : public Component
{
public:
	PlayerController();
	PlayerController(SharedPtr<Entity> owner);

	virtual void OnCreate() override;
	virtual void OnUpdate(const float& deltaTime) override;

	virtual void Serialize(SharedPtr<Serialization::SerializableElement> element, bool force = false) override;
	virtual void Deserialize(SharedPtr<Serialization::SerializableElement> element, bool force = false) override;

private:
	glm::mat2 mat2;
	glm::mat3 mat3;

	REGISTER_DEC_TYPE(PlayerController);
};