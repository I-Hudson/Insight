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
            const SkinnedMeshComponent* skinnedMeshComponent = owner->GetComponent<SkinnedMeshComponent>();
            if (!skinnedMeshComponent)
            {
                return;
            }
            m_animator.SetSkelton(skinnedMeshComponent->GetSkeleton());
            m_animator.Update(delta_time * m_playbackScale);
        }
    }
}