#include "ECS/Components/AnimationClipComponent.h"
#include "ECS/Components/SkinnedMeshComponent.h"

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
            m_animator.SetAnimationClip(m_animationClip);
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
                m_animator.SetSkelton(m_skeleton);
            }
        }

        Ref<Runtime::Skeleton> AnimationClipComponent::GetSkeleton() const
        {
            return m_skeleton;
        }

        Runtime::Animator& AnimationClipComponent::GetAnimator()
        {
            return m_animator;
        }

        const Runtime::Animator& AnimationClipComponent::GetAnimator() const
        {
            return m_animator;
        }

        void AnimationClipComponent::OnUpdate(const float delta_time)
        {
            const ECS::Entity* owner = GetOwnerEntity();
            ASSERT(owner);
            m_animator.Update(delta_time * m_playbackScale);
        }
    }
}