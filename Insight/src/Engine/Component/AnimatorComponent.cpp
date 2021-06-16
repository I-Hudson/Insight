#include "ispch.h"
#include "Engine/Component/AnimatorComponent.h"
#include "glm/gtx/quaternion.hpp"

AnimatorComponent::AnimatorComponent()
    : m_currentAnimation(nullptr)
    , m_currentTime(0.0f)
    , m_deltaTime(0.0f)
{
    SetType<AnimatorComponent>();
}

AnimatorComponent::AnimatorComponent(ComponentManager * componentManager, ComponentID componentID, ComponentType componentType, EntityManager * entityManager, EntityID entity)
    : Component(componentManager, componentID, componentType, entityManager, entity)
    , m_currentAnimation(nullptr)
    , m_currentTime(0.0f)
    , m_deltaTime(0.0f)
{
    SetType<AnimatorComponent>();
}

AnimatorComponent::~AnimatorComponent()
{}

void AnimatorComponent::OnUpdate(const float& a_deltaTime)
{
    IS_PROFILE_FUNCTION();
    m_deltaTime = a_deltaTime;
    if (m_currentAnimation && m_currentSkeleton)
    {
        m_currentTime += m_currentAnimation->GetPlayBackSpeed() * m_currentAnimation->GetTicksPerSecond() * m_deltaTime;
        m_currentTime = fmod(m_currentTime, m_currentAnimation->GetDuration());
        CalculateBoneTransform();
        ApplyBoneToModelSpace(m_currentSkeleton->GetRootBone(), glm::mat4(1));
    }
}

void AnimatorComponent::SetSkelton(Insight::Animation::Skeleton* skeleton)
{
    m_currentSkeleton = skeleton;
    Reset();
    // Validate that the new skeleton is compatible with the animations.
}

void AnimatorComponent::PlayAnimation(Insight::Animation::Animation* animation)
{
	m_currentAnimation = animation;
	m_currentTime = 0.0f;
}

void AnimatorComponent::Reset()
{
    m_finalBoneMatrices.clear();
    m_finalBoneMatrices.resize(m_currentSkeleton->GetBoneCount());
    for (u32 i = 0; i < m_currentSkeleton->GetBoneCount(); ++i)
    {
        m_finalBoneMatrices.at(i) = glm::mat4(1.0f);
    }
}

void AnimatorComponent::CalculateBoneTransform()
{
    const std::vector<Insight::Animation::Bone>& skeltonBones = m_currentSkeleton->GetAllBones();
    for (auto& b : skeltonBones)
    {
        Insight::Animation::KeyPosition pPosition = m_currentAnimation->GetPreviousPositionKey(b.GetBoneName(), m_currentTime);
        Insight::Animation::KeyPosition nPosition = m_currentAnimation->GetNextPositionKey(b.GetBoneName(), m_currentTime);

        Insight::Animation::KeyRotation pRotation = m_currentAnimation->GetPreviousRotationKey(b.GetBoneName(), m_currentTime);
        Insight::Animation::KeyRotation nRotation = m_currentAnimation->GetNextRotationKey(b.GetBoneName(), m_currentTime);

        Insight::Animation::KeyScale pScale = m_currentAnimation->GetPreviousScaleKey(b.GetBoneName(), m_currentTime);
        Insight::Animation::KeyScale nScale = m_currentAnimation->GetNextScaleKey(b.GetBoneName(), m_currentTime);

        float animationProgess = m_currentAnimation->CalculateProgressionThroughFrame(m_currentTime, pPosition.TimeStamp, nPosition.TimeStamp);
        glm::mat4 boneTransform = InterpolateTransform(pPosition, nPosition, pRotation, nRotation, pScale, nScale, animationProgess);
        m_finalBoneMatrices[b.GetBoneId()] = boneTransform;
    }
}

void AnimatorComponent::ApplyBoneToModelSpace(Insight::Animation::Bone& bone, glm::mat4 parentTransform)
{
    glm::mat4 currentLocalTransform = m_finalBoneMatrices.at(bone.GetBoneId());
    glm::mat4 currentTransform = parentTransform * currentLocalTransform;
    for (u32 i = 0; i < bone.GetChildrenCount(); ++i)
    {
        Insight::Animation::Bone& cBone = m_currentSkeleton->GetBone(bone.GetChildren()[i]);
        ApplyBoneToModelSpace(cBone, currentTransform);
    }
    currentTransform = currentTransform * bone.GetLocalBindTransform();
    m_finalBoneMatrices.at(bone.GetBoneId()) = currentTransform;
}

glm::mat4 AnimatorComponent::InterpolateTransform(const Insight::Animation::KeyPosition& pPosition, const Insight::Animation::KeyPosition& nPosition, 
                                                  const Insight::Animation::KeyRotation& pRotation, const Insight::Animation::KeyRotation& nRotation, 
                                                  const Insight::Animation::KeyScale& pScale, const Insight::Animation::KeyScale& nScale, const float& progress)
{
    glm::mat4 position = glm::translate(glm::mat4(1), glm::mix(pPosition.Position, nPosition.Position, progress));
    glm::mat4 rotation = glm::toMat4(glm::slerp(pRotation.Orientation, nRotation.Orientation, progress));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::mix(pScale.Scale, nScale.Scale, progress));

    return position * rotation * scale;
}
