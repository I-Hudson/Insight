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

void PlayerController::OnCreate()
{
	__super::OnCreate();
	mat2 = glm::mat2();
	mat3 = glm::mat3();

	IS_REGISTER_PROPERTY(glm::mat2, mat2, "Test Matrix 2", ShowInEditor | ClampZero);
	IS_REGISTER_PROPERTY(glm::mat3, mat3, "Test Matrix 4", ShowInEditor | ReadOnly);
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


void PlayerController::Serialize(Insight::Serialization::SerializableElement* element, bool force)
{
	//tinyxml2::XMLElement* Type = doc->NewElement("Type");
	//Type->SetText("PlayerController");
	//out->InsertEndChild(Type);
}

void PlayerController::Deserialize(Insight::Serialization::SerializableElement* element, bool force)
{
}
