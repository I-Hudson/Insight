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

	void SetSkelton(Insight::Animation::Skeleton* skeleton);
	void PlayAnimation(Insight::Animation::Animation* animation);

	Insight::Animation::Skeleton* GetSkelton() const { return m_currentSkeleton; }
	Insight::Animation::Animation* GetCurrentAnimation() const { return m_currentAnimation; }

	std::vector<glm::mat4> GetFinalBoneMatrices() const { return m_finalBoneMatrices; }

private:
	void Reset();
	void CalculateBoneTransform();
	void ApplyBoneToModelSpace(Insight::Animation::Bone& bone, glm::mat4 parentTransform);
	glm::mat4 InterpolateTransform(const Insight::Animation::KeyPosition& pPosition, const Insight::Animation::KeyPosition& nPosition,
								   const Insight::Animation::KeyRotation& pRotation, const Insight::Animation::KeyRotation& nRotation,
								   const Insight::Animation::KeyScale& pScale, const Insight::Animation::KeyScale& nScale, const float& progress);

private:
	std::vector<glm::mat4> m_finalBoneMatrices;
	Insight::Animation::Skeleton* m_currentSkeleton = nullptr;
	Insight::Animation::Animation* m_currentAnimation = nullptr;
	float m_currentTime;
	float m_deltaTime;	
};