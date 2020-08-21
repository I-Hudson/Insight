#pragma once

#include "Insight/Component/Component.h"

class PlayerController : public Component
{
public:
	PlayerController();
	PlayerController(Entity* owner);

	virtual void OnUpdate(const float& deltaTime) override;

	virtual void Serialize(json& out, bool force = false) override;
	virtual void Deserialize(json in, bool force = false) override;

private:
	REGISTER_DEC_TYPE(PlayerController);
};