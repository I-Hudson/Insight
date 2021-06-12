#include "ispch.h"
#include "Engine/Component/AnimatorComponent.h"

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
    if (m_currentAnimation)
    {
        m_currentTime += m_currentAnimation->GetTicksPerSecond() * m_deltaTime;
        m_currentTime = fmod(m_currentTime, m_currentAnimation->GetDuration());
        CalculateBoneTransform(m_currentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
}

void AnimatorComponent::PlayAnimation(Insight::Animation::Animation* animation)
{
    Reset(animation);
	m_currentAnimation = animation;
	m_currentTime = 0.0f;
}

void AnimatorComponent::Reset(Insight::Animation::Animation* animation)
{
    if (animation == m_currentAnimation)
    {
        return;
    }

    m_finalBoneMatrices.clear();
    m_finalBoneMatrices.resize(animation->GetBoneCount());
    for (u32 i = 0; i < animation->GetBoneCount(); ++i)
    {
        m_finalBoneMatrices.at(i) = glm::mat4(1.0f);
    }
}

void AnimatorComponent::CalculateBoneTransform(const Insight::Animation::NodeData& node, glm::mat4 parentTransform)
{
    std::string nodeName = node.Name;
    glm::mat4 nodeTransform = node.Transform;

    Insight::Animation::Bone* Bone = m_currentAnimation->FindBone(nodeName);

    if (Bone)
    {
        Bone->Update(m_currentTime);
        nodeTransform = Bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_currentAnimation->GetBoneInfoMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].Id;
        glm::mat4 offset = boneInfoMap[nodeName].Offset;
        m_finalBoneMatrices[index] = globalTransformation * offset;
    }

    for (u32 i = 0; i < node.ChildrenCount; ++i)
    {
        CalculateBoneTransform(node.Children[i], globalTransformation);
    }
}
