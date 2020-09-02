#pragma once

#include "Insight/Component/Component.h"

class PlayerController : public Component
{
public:
	PlayerController();
	PlayerController(Entity* owner);

	virtual void OnUpdate(const float& deltaTime) override;

	virtual void Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force = false) override;
	virtual void Deserialize(tinyxml2::XMLNode* data, bool force = false) override;

private:
	REGISTER_DEC_TYPE(PlayerController);
};