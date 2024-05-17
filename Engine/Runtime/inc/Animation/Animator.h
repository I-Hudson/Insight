#pragma once

#include "Runtime/Defines.h"

#include "Core/ReferencePtr.h"
#include "Resource/Skeleton.h"
#include "Resource/AnimationClip.h"

namespace Insight
{
    namespace Runtime
    {
        class IS_RUNTIME Animator : Core::RefCount
        {
        public:
            Animator();
            virtual ~Animator() override;

            Ref<Skeleton> GetSkelton() const;
            void SetSkelton(Ref<Skeleton> skeleton);

            Ref<AnimationClip> GetAnimationClip() const;
            void SetAnimationClip(Ref<AnimationClip> animationClip);

            void Update(const float deltaTime);

            const std::vector<Maths::Matrix4>& GetBoneTransforms() const;

        private:
            void CalculateBoneTransform(const u32 boneId, const Maths::Matrix4 parentTransform);
            void CalculateBoneTransform(std::string_view nodeName, const Maths::Matrix4 parentTransform);

            /// @brief Gets normalized value for Lerp & Slerp
            /// @param lastTimeStamp 
            /// @param nextTimeStamp 
            /// @return float
            float GetScaleFactor(const double lastTimeStamp, const double nextTimeStamp) const;

            Maths::Matrix4 InterpolatePosition(const u32 boneId) const;
            Maths::Matrix4 InterpolateRotation(const u32 boneId) const;
            Maths::Matrix4 InterpolateScale(const u32 boneId) const;

            /// @brief Reset our animation time as either the skeleton or animation clip has changed.
            void Reset();

        private:
            Ref<Skeleton> m_skelton;
            Ref<AnimationClip> m_animationClip;

            /// @brief Final transforms for all bones.
            std::vector<Maths::Matrix4> m_boneMatrices;
        
            double m_currentAnimationTime = 0.0f;
            float m_deltaTime = 0.0f;
        };
    }
}