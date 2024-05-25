#pragma once

#include "Core/ISysytem.h"
#include "Core/Singleton.h"
#include "Core/GUID.h"

#include "Animation/Animator.h"

namespace Insight
{
    namespace Runtime
    {
        /// @brief Store a singe instance of something that is being animated by the animation system.
        struct AnimationInstance
        {
            Animator Animator;
            Core::GUID OwnerObject;
        };

        class AnimationSystem : public Core::ISystem, public Core::Singleton<AnimationSystem>
        {
        public:
            AnimationSystem();
            virtual ~AnimationSystem() override;

            IS_SYSTEM(AnimationSystem);

            void Update(const float deltaTime);

            AnimationInstance* AddAnimationInstance(const Core::GUID& guid);
            void RemoveAnimationInstance(const Core::GUID& guid);

            AnimationInstance* GetAnimationInstance(const Core::GUID& guid);
            const AnimationInstance* GetAnimationInstance(const Core::GUID& guid) const;

        private:
            std::vector<AnimationInstance> m_animations;
            mutable std::mutex m_animationsLock;
        };
    }
}