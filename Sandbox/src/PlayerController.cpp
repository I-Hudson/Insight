#include "PlayerController.h"

#include "Insight/Component/TransformComponent.h"
#include "Insight/Time/Time.h"
#include "Insight/Input/Input.h"

REGISTER_DEF_TYPE(PlayerController);

PlayerController::PlayerController()
	: Component(nullptr)
{
	m_componentId = GetComponentID<PlayerController>();
}

PlayerController::PlayerController(Entity* owner)
	: Component(owner)
{
	m_componentId = GetComponentID<PlayerController>();
}

void PlayerController::OnUpdate(const float& deltaTime)
{
	if (Input::KeyDown(KEY_UP))
	{
		GetComponent<TransformComponent>()->SetPosition(GetComponent<TransformComponent>()->GetPostion() + glm::vec3(0.0f, 0.0f, 5.0f * Insight::Time::GetDeltaTime()));
	}

	if (Input::KeyDown(KEY_DOWN))
	{
		GetComponent<TransformComponent>()->SetPosition(GetComponent<TransformComponent>()->GetPostion() - glm::vec3(0.0f, 0.0f, 5.0f * Insight::Time::GetDeltaTime()));
	}

	if (Input::KeyDown(KEY_LEFT))
	{
		GetComponent<TransformComponent>()->SetPosition(GetComponent<TransformComponent>()->GetPostion() + glm::vec3(5.0f * Insight::Time::GetDeltaTime(), 0.0f, 0.0f));
	}

	if (Input::KeyDown(KEY_RIGHT))
	{
		GetComponent<TransformComponent>()->SetPosition(GetComponent<TransformComponent>()->GetPostion() - glm::vec3(5.0f * Insight::Time::GetDeltaTime(), 0.0f, 0.0f));
	}
}

void PlayerController::Serialize(json& out, bool force)
{
	__super::Serialize(out);
	out["Type"] = "PlayerController";
}

void PlayerController::Deserialize(json in, bool force)
{
	__super::Deserialize(in);
}
