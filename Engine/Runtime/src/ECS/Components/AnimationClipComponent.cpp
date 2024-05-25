#include "ECS/Components/AnimationClipComponent.h"
#include "Animation/AnimationSystem.h"

namespace Insight
{
    namespace ECS
    {
        AnimationClipComponent::AnimationClipComponent()
        { }
        AnimationClipComponent::~AnimationClipComponent()
        { }

        IS_SERIALISABLE_CPP(AnimationClipComponent);

        void AnimationClipComponent::SetAnimationClip(Ref<Runtime::AnimationClip> animationClip)
        {
            m_animationClip = animationClip;
            GetAnimator()->SetAnimationClip(m_animationClip);
        }

        Ref<Runtime::AnimationClip> AnimationClipComponent::GetAnimationClip() const
        {
            return m_animationClip;
        }

        void AnimationClipComponent::SetSkeleton(Ref<Runtime::Skeleton> skeleton)
        {
            if (m_skeleton != skeleton)
            {
                m_skeleton = skeleton;
                GetAnimator()->SetSkelton(m_skeleton);
            }
        }

        Ref<Runtime::Skeleton> AnimationClipComponent::GetSkeleton() const
        {
            return m_skeleton;
        }

        Runtime::Animator* AnimationClipComponent::GetAnimator()
        {
            Runtime::AnimationInstance* animInstance = Runtime::AnimationSystem::Instance().GetAnimationInstance(GetGuid());
            ASSERT(animInstance);
            return &animInstance->Animator;
        }

        const Runtime::Animator* AnimationClipComponent::GetAnimator() const
        {
            const Runtime::AnimationInstance* animInstance = Runtime::AnimationSystem::Instance().GetAnimationInstance(GetGuid());
            ASSERT(animInstance);
            return &animInstance->Animator;
        }

        void AnimationClipComponent::OnCreate()
        {
            Runtime::AnimationSystem::Instance().AddAnimationInstance(GetGuid());
        }

        void AnimationClipComponent::OnDestroy()
        {
            Runtime::AnimationSystem::Instance().RemoveAnimationInstance(GetGuid());
        }
    }
}