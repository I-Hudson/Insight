#pragma once

#include "Runtime/Defines.h"

#include "Core/ReferencePtr.h"
#include "Resource/Skeleton.h"
#include "Resource/AnimationClip.h"

namespace Insight
{
    namespace Runtime
    {
        class IS_RUNTIME Animator : public Core::RefCount
        {
        public:
            Animator();
            virtual ~Animator() override;

            Ref<Skeleton> GetSkelton() const;
            void SetSkelton(Ref<Skeleton> skeleton);

            Ref<AnimationClip> GetAnimationClip() const;
            void SetAnimationClip(Ref<AnimationClip> animationClip);

            void Update(const float deltaTime);
            void Play(const bool resetClip = false);
            void Stop();

            const std::vector<Maths::Matrix4>& GetBoneTransforms() const;

        private:
            void CalculateBoneTransform(const u32 boneId, const Maths::Vector3 parentPosition, const Maths::Quaternion parentQuaternion, const Maths::Vector3 parentScale);
            void CalculateBoneTransform(const u32 boneId, const Maths::Matrix4 parentTransform);
#if ANIMATION_NODE_TRANSFORMS
            void CalculateBoneTransform(const AnimationNode* node, const Maths::Matrix4 parentTransform);
#endif
            /// @brief Gets normalized value for Lerp & Slerp
            /// @param lastTimeStamp 
            /// @param nextTimeStamp 
            /// @return float
            float GetScaleFactor(const double lastTimeStamp, const double nextTimeStamp) const;

            Maths::Matrix4 InterpolatePosition(const u32 boneId) const;
            Maths::Matrix4 InterpolateRotation(const u32 boneId) const;
            Maths::Matrix4 InterpolateScale(const u32 boneId) const;

            Maths::Vector3 InterpolatePositionVec(const u32 boneId) const;
            Maths::Quaternion InterpolateRotationQuat(const u32 boneId) const;
            Maths::Vector3 InterpolateScaleVec(const u32 boneId) const;

            /// @brief Reset our animation time as either the skeleton or animation clip has changed.
            void Reset();
            void SetBindPose();

        private:
            Ref<Skeleton> m_skelton;
            Ref<AnimationClip> m_animationClip;

            /// @brief Final transforms for all bones.
            std::vector<Maths::Matrix4> m_boneMatrices;
        
            double m_currentAnimationTime = 0.0f;
            float m_deltaTime = 0.0f;
            bool m_isPlaying = true;
        };
    }
}