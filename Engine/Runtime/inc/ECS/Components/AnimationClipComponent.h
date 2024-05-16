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

            Runtime::Animator& GetAnimator();
            const Runtime::Animator& GetAnimator() const;

            virtual void OnUpdate(const float delta_time) override;

        private:
            Runtime::Animator m_animator;
            Ref<Runtime::AnimationClip> m_animationClip;
            REFLECT_PROPERTY(EditorVisible)
            float m_playbackScale = 1.0f;
        };
    }

    OBJECT_SERIALISER(ECS::AnimationClipComponent, 1,
        SERIALISE_BASE(ECS::Component, 1, 0)
        //SERIALISE_COMPLEX(Serialisation::AnimationClipToGuid, m_animationClip, 1, 0)
        SERIALISE_PROPERTY(float, m_playbackScale, 1, 0)
    );
}