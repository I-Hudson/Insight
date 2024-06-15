#include "Animation/Animator.h"
#include "Core/Profiler.h"
#include "Core/Asserts.h"

namespace Insight
{
    namespace Runtime
    {
        Animator::Animator()
        { }
        Animator::~Animator()
        { }

        Ref<Skeleton> Animator::GetSkelton() const
        {
            return m_skelton;
        }

        void Animator::SetSkelton(Ref<Skeleton> skeleton)
        {
            if (m_skelton != skeleton)
            {
                m_skelton = skeleton;
                m_boneMatrices.resize(m_skelton->GetNumberOfBones(), Maths::Matrix4::Identity);
                Reset();
                SetBindPose();
            }
        }

        Ref<AnimationClip> Animator::GetAnimationClip() const
        {
            return m_animationClip;
        }

        void Animator::SetAnimationClip(Ref<AnimationClip> animationClip)
        {
            if (m_animationClip != animationClip)
            {
                m_animationClip = animationClip;
                Reset();
            }
        }

        void Animator::Update(const float deltaTime)
        {
            IS_PROFILE_FUNCTION();

            if (m_skelton && m_animationClip)
            {
                m_currentAnimationTime += m_animationClip->GetTickPerSecond() * static_cast<double>(deltaTime);
                m_currentAnimationTime = fmod(m_currentAnimationTime, m_animationClip->GetDuration());
                CalculateBoneTransform(m_skelton->GetRootBone().Id, Maths::Matrix4::Identity);
#if ANIMATION_NODE_TRANSFORMS
                //CalculateBoneTransform(&m_animationClip->GetRootNode(), Maths::Matrix4::Identity);
#endif
            }
        }

        const std::vector<Maths::Matrix4>& Animator::GetBoneTransforms() const
        {
            return m_boneMatrices;
        }

        void Animator::CalculateBoneTransform(const u32 boneId, const Maths::Vector3 parentPosition, const Maths::Quaternion parentQuaternion, const Maths::Vector3 parentScale)
        {
            IS_PROFILE_FUNCTION();

            const SkeletonBone& bone = m_skelton->GetBone(boneId);
            ASSERT(bone);

            const Maths::Vector3 bonePositionVector = InterpolatePositionVec(boneId);
            const Maths::Quaternion boneRotationQuat = InterpolateRotationQuat(boneId);
            const Maths::Vector3 boneScaleVector = InterpolateScaleVec(boneId);

            const Maths::Vector3 globalPosition = parentPosition * bonePositionVector;
            const Maths::Quaternion globalRotation = parentQuaternion * boneRotationQuat;
            const Maths::Vector3 globalScale = parentScale * boneScaleVector;

            const Maths::Matrix4 globalTransform = Maths::Matrix4::Identity
                .Scaled(Maths::Vector4(globalScale, 1.0f))
                    .Rotated(globalRotation)
                    .Translated(Maths::Vector4(globalPosition, 1.0f));

            const Maths::Matrix4 boneOffsetTransform = m_skelton->GetGlobalInverseTransform() * globalTransform * bone.Offset;
            m_boneMatrices[boneId] = boneOffsetTransform;

            for (size_t childBoneIdx = 0; childBoneIdx < bone.ChildrenBoneIds.size(); ++childBoneIdx)
            {
                const u32 childBoneId = bone.ChildrenBoneIds[childBoneIdx];
                CalculateBoneTransform(childBoneId, globalPosition, globalRotation, globalScale);
            }
        }

        void Animator::CalculateBoneTransform(const u32 boneId, const Maths::Matrix4 parentTransform)
        {
            IS_PROFILE_FUNCTION();

            const SkeletonBone& bone = m_skelton->GetBone(boneId);
            ASSERT(bone);

            const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(boneId);
            const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(boneId);
            const Maths::Matrix4 boneScaleMatrix = InterpolateScale(boneId);
            const Maths::Matrix4 boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;

            const Maths::Matrix4 globalTransform = parentTransform * boneTransform;

            const Maths::Matrix4 boneOffsetTransform = m_skelton->GetGlobalInverseTransform() * globalTransform * bone.Offset;
            m_boneMatrices[boneId] = boneOffsetTransform;

            for (size_t childBoneIdx = 0; childBoneIdx < bone.ChildrenBoneIds.size(); ++childBoneIdx)
            {
                const u32 childBoneId = bone.ChildrenBoneIds[childBoneIdx];
                CalculateBoneTransform(childBoneId, globalTransform);
            }
        }

#if ANIMATION_NODE_TRANSFORMS
        void Animator::CalculateBoneTransform(const AnimationNode* node, const Maths::Matrix4 parentTransform)
        {
            IS_PROFILE_FUNCTION();

            const AnimationBoneTrack* bone = m_animationClip->GetBoneTrack(node->Name);

            Maths::Matrix4 nodeTransform = node->Transform;
            if (bone)
            {
                const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(bone->BoneId);
                const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(bone->BoneId);
                const Maths::Matrix4 boneScaleMatrix = InterpolateScale(bone->BoneId);


                const Maths::Matrix4 boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;
                nodeTransform = boneTransform;
            }

            const Maths::Matrix4 globalTransform = parentTransform * nodeTransform;

            auto boneInfoMap = m_animationClip->GetBoneIDMap();
            if (boneInfoMap.find(node->Name) != boneInfoMap.end())
            {
                int index = boneInfoMap[node->Name].Id;
                Maths::Matrix4 offset = boneInfoMap[node->Name].Offset;
                m_boneMatrices[index] = globalTransform * offset;
            }

            /*
            if (bone)
            {
                const Maths::Matrix4 boneOffsetTransform = globalTransform * bone.Offset;
                m_boneMatrices[bone.Id] = boneOffsetTransform;
            }
            */
            for (size_t childBoneIdx = 0; childBoneIdx < node->ChildrenCount; ++childBoneIdx)
            {
                CalculateBoneTransform(&node->Children[childBoneIdx], globalTransform);
            }
        }
#endif

        float Animator::GetScaleFactor(const double lastTimeStamp, const double nextTimeStamp) const
        {
            float scaleFactor = 0.0f;
            float midWayLength = m_currentAnimationTime - static_cast<float>(lastTimeStamp);
            float framesDiff = nextTimeStamp - static_cast<float>(lastTimeStamp);
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        Maths::Matrix4 Animator::InterpolatePosition(const u32 boneId) const
        {
            return Maths::Matrix4::Identity.Translated(Maths::Vector4(InterpolatePositionVec(boneId), 1.0f));
        }

        Maths::Matrix4 Animator::InterpolateRotation(const u32 boneId) const
        {
            return Maths::Matrix4(InterpolateRotationQuat(boneId));
        }

        Maths::Matrix4 Animator::InterpolateScale(const u32 boneId) const
        {
            return Maths::Matrix4::Identity.Scaled(Maths::Vector4(InterpolateScaleVec(boneId), 1.0f));
        }

        Maths::Vector3 Animator::InterpolatePositionVec(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Vector3::Zero;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Vector3::Zero;
            }
            else if (boneTrack->Positions.size() == 1)
            {
                const Maths::Vector4 position = Maths::Vector4(boneTrack->Positions[0].Position, 1.0f);
                return position;
            }

            const u32 p0Index = boneTrack->GetPositionKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::PositionKeyFrame& p0KeyFrame = boneTrack->Positions[p0Index];
            const AnimationBoneTrack::PositionKeyFrame& p1KeyFrame = boneTrack->Positions[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalPosition = p0KeyFrame.Position.Lerp(p1KeyFrame.Position, scaleFactor);

            return finalPosition;
        }

        Maths::Quaternion Animator::InterpolateRotationQuat(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Quaternion::Identity;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Quaternion::Identity;
            }
            else if (boneTrack->Rotations.size() == 1)
            {
                return boneTrack->Rotations[0].Rotation.Normalised();
            }

            const u32 p0Index = boneTrack->GetRotationKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::RotationKeyFrame& p0KeyFrame = boneTrack->Rotations[p0Index];
            const AnimationBoneTrack::RotationKeyFrame& p1KeyFrame = boneTrack->Rotations[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Quaternion finalRotation = p0KeyFrame.Rotation.Slerp(p1KeyFrame.Rotation, scaleFactor);

            return finalRotation.Normalised();
        }

        Maths::Vector3 Animator::InterpolateScaleVec(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Vector3::One;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Vector3::One;
            }
            else if (boneTrack->Scales.size() == 1)
            {
                const Maths::Vector3 scale = boneTrack->Scales[0].Scale;
                return scale;
            }

            const u32 p0Index = boneTrack->GetScaleKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::ScaleKeyFrame& p0KeyFrame = boneTrack->Scales[p0Index];
            const AnimationBoneTrack::ScaleKeyFrame& p1KeyFrame = boneTrack->Scales[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalScale = p0KeyFrame.Scale.Lerp(p1KeyFrame.Scale, scaleFactor);

            return finalScale;
        }

        void Animator::Reset()
        {
            m_currentAnimationTime = 0;
        }

        void Animator::SetBindPose()
        {
            const u32 boneSize = m_skelton->GetNumberOfBones();
            for (size_t i = 0; i < boneSize; ++i)
            {
                const SkeletonBone& bone = m_skelton->GetBone(i);
                m_boneMatrices[bone.Id] = Maths::Matrix4::Identity;
            }
        }
    }
}