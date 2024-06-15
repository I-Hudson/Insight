#pragma once

#include "Runtime/Defines.h"
#include "ECS/Entity.h"

#include "Resource/AnimationClip.h"
#include "Animation/Animator.h"

#include "Generated/AnimationClipComponent_reflect_generated.h"

namespace Insight
{
    namespace ECS
    {
        REFLECT_CLASS();
        /// @brief Component used to play a single animation clip.
        class IS_RUNTIME AnimationClipComponent : public Component
        {
            REFLECT_GENERATED_BODY();
        public:
            IS_COMPONENT(AnimationClipComponent);

            AnimationClipComponent();
            virtual ~AnimationClipComponent() override;

            IS_SERIALISABLE_H(AnimationClipComponent);

            void SetAnimationClip(Ref<Runtime::AnimationClip> animationClip);
            Ref<Runtime::AnimationClip> GetAnimationClip() const;

            void SetSkeleton(Ref<Runtime::Skeleton> skeleton);
            Ref<Runtime::Skeleton> GetSkeleton() const;

            Runtime::Animator* GetAnimator();
            const Runtime::Animator* GetAnimator() const;

            /// Component - Begin
            virtual void OnCreate() override;
            virtual void OnDestroy() override;
            /// Component - End

        private:
            Ref<Runtime::AnimationClip> m_animationClip;
            Ref<Runtime::Skeleton> m_skeleton;

            REFLECT_PROPERTY(EditorVisible)
            float m_playbackScale = 1.0f;
        };
    }

    OBJECT_SERIALISER(ECS::AnimationClipComponent, 1,
        SERIALISE_BASE(ECS::Component, 1, 0)
        //SERIALISE_COMPLEX(Serialisation::AnimationClipToGuid, m_animationClip, 1, 0)
        //SERIALISE_COMPLEX(Serialisation::AnimationClipToGuid, m_animationClip, 1, 0)
        SERIALISE_PROPERTY(float, m_playbackScale, 1, 0)
    );
}