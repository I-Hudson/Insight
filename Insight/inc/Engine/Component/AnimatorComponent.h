#pragma once

#include "Engine/Component/Component.h"
#include "Engine/Model/Animation.h"

class AnimatorComponent : public Component
{
public:
	AnimatorComponent();
	AnimatorComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~AnimatorComponent() override;

	virtual void OnUpdate(const float& a_deltaTime);

	void PlayAnimation(Insight::Animation::Animation* animation);

	std::vector<glm::mat4> GetFinalBoneMatrices() const { return m_finalBoneMatrices; }

private:
	void Reset(Insight::Animation::Animation* animation);
	void CalculateBoneTransform(const Insight::Animation::NodeData& node, glm::mat4 parentTransform);


private:
	std::vector<glm::mat4> m_finalBoneMatrices;
	Insight::Animation::Animation* m_currentAnimation;
	float m_currentTime;
	float m_deltaTime;	
};