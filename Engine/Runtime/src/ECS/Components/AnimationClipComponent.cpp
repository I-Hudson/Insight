#include "ECS/Components/AnimationClipComponent.h"
#include "ECS/Components/AnimationClipComponent.inl"
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

        void AnimationClipComponent::Play(const bool resetClip)
        {
            GetAnimator()->Play(resetClip);
        }

        void AnimationClipComponent::Stop()
        {
            GetAnimator()->Stop();
        }

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
            Runtime::AnimationInstance* animInstance = Runtime::AnimationSystem::Instance().GetAnimationInstance(GetOwnerEntity());
            ASSERT(animInstance);
            return &animInstance->Animator;
        }

        const Runtime::Animator* AnimationClipComponent::GetAnimator() const
        {
            const Runtime::AnimationInstance* animInstance = Runtime::AnimationSystem::Instance().GetAnimationInstance(GetOwnerEntity());
            ASSERT(animInstance);
            return &animInstance->Animator;
        }

        void AnimationClipComponent::OnCreate()
        {
            Runtime::AnimationSystem::Instance().AddAnimationInstance(GetOwnerEntity());

            OnEnabled.Bind<&AnimationClipComponent::OnEnabledCallback>(this);
        }

        void AnimationClipComponent::OnDestroy()
        {
            Runtime::AnimationSystem::Instance().RemoveAnimationInstance(GetOwnerEntity());
            OnEnabled.Unbind<&AnimationClipComponent::OnEnabledCallback>(this);
        }

        void AnimationClipComponent::OnEnabledCallback(const bool enabled) const
        {
            if (enabled)
            {
                Runtime::AnimationSystem::Instance().AddAnimationInstance(GetOwnerEntity());
            }
            else
            {
                Runtime::AnimationSystem::Instance().RemoveAnimationInstance(GetOwnerEntity());
            }
        }
    }
}