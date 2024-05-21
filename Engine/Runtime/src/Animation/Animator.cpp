#include "Animation/Animator.h"

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
            if (m_skelton && m_animationClip)
            {
                //m_currentAnimationTime += m_animationClip->GetTickPerSecond() * static_cast<double>(deltaTime);
                m_currentAnimationTime = fmod(m_currentAnimationTime, m_animationClip->GetDuration());
                CalculateBoneTransform(m_skelton->GetRootNode().Name, Maths::Matrix4::Identity);
            }
        }

        const std::vector<Maths::Matrix4>& Animator::GetBoneTransforms() const
        {
            return m_boneMatrices;
        }

        void Animator::CalculateBoneTransform(const u32 boneId, const Maths::Matrix4 parentTransform)
        {
            const SkeletonBone& bone = m_skelton->GetBone(boneId);
            ASSERT(bone);

            const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(boneId);
            const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(boneId);
            const Maths::Matrix4 boneScaleMatrix = InterpolateScale(boneId);

            const Maths::Matrix4 boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;
            const Maths::Matrix4 globalTransform = parentTransform * boneTransform;

            const Maths::Matrix4 boneOffsetTransform = globalTransform * bone.Offset;
            m_boneMatrices[boneId] = boneOffsetTransform;

            for (size_t childBoneIdx = 0; childBoneIdx < bone.ChildrenBoneIds.size(); ++childBoneIdx)
            {
                const u32 childBoneId = bone.ChildrenBoneIds[childBoneIdx];
                CalculateBoneTransform(childBoneId, globalTransform);
            }
        }

        void Animator::CalculateBoneTransform(std::string_view nodeName, const Maths::Matrix4 parentTransform)
        {
            SkeletonNode* node = m_skelton->GetNode(nodeName);
            const SkeletonBone& bone = m_skelton->GetBone(nodeName);

            Maths::Matrix4 nodeTransform = node->Transforms;
            if (bone)
            {
                const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(bone.Id);
                const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(bone.Id);
                const Maths::Matrix4 boneScaleMatrix = InterpolateScale(bone.Id);

                const Maths::Matrix4 boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;
                nodeTransform = boneTransform;
            }

            const Maths::Matrix4 globalTransform = parentTransform * nodeTransform;

            if (bone)
            {
                const Maths::Matrix4 boneOffsetTransform = globalTransform * bone.Offset;
                m_boneMatrices[bone.Id] = boneOffsetTransform;
            }

            for (size_t childBoneIdx = 0; childBoneIdx < node->ChildrenNames.size(); ++childBoneIdx)
            {
                CalculateBoneTransform(node->ChildrenNames[childBoneIdx], globalTransform);
            }
        }

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
            if (!m_animationClip)
            {
                return Maths::Matrix4::Identity;
            }
            
            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Matrix4::Identity;
            }
            else if (boneTrack->Positions.size() == 1)
            {
                const Maths::Vector4 position = Maths::Vector4(boneTrack->Positions[0].Position, 1.0f);
                return Maths::Matrix4::Identity.Translated(position);
            }

            const u32 p0Index = boneTrack->GetPositionKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::PositionKeyFrame& p0KeyFrame = boneTrack->Positions[p0Index];
            const AnimationBoneTrack::PositionKeyFrame& p1KeyFrame = boneTrack->Positions[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalPosition = p0KeyFrame.Position.Lerp(p1KeyFrame.Position, scaleFactor);

            return Maths::Matrix4::Identity.Translated(Maths::Vector4(finalPosition, 1.0f));
        }

        Maths::Matrix4 Animator::InterpolateRotation(const u32 boneId) const
        {
            if (!m_animationClip)
            {
                return Maths::Matrix4::Identity;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Matrix4::Identity;
            }
            else if (boneTrack->Rotations.size() == 1)
            {
                return Maths::Matrix4(boneTrack->Rotations[0].Rotation.Normalised());
            }

            const u32 p0Index = boneTrack->GetRotationKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::RotationKeyFrame& p0KeyFrame = boneTrack->Rotations[p0Index];
            const AnimationBoneTrack::RotationKeyFrame& p1KeyFrame = boneTrack->Rotations[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Quaternion finalRotation = p0KeyFrame.Rotation.Slerp(p1KeyFrame.Rotation, scaleFactor);

            return Maths::Matrix4(finalRotation.Normalised());
        }

        Maths::Matrix4 Animator::InterpolateScale(const u32 boneId) const
        {
            if (!m_animationClip)
            {
                return Maths::Matrix4::Identity;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Matrix4::Identity;
            }
            else if (boneTrack->Scales.size() == 1)
            {
                const Maths::Vector4 scale = Maths::Vector4(boneTrack->Scales[0].Scale, 1.0f);
                return Maths::Matrix4::Identity.Scaled(scale);
            }

            const u32 p0Index = boneTrack->GetScaleKeyFrameIndex(m_currentAnimationTime);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::ScaleKeyFrame& p0KeyFrame = boneTrack->Scales[p0Index];
            const AnimationBoneTrack::ScaleKeyFrame& p1KeyFrame = boneTrack->Scales[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalScale = p0KeyFrame.Scale.Lerp(p1KeyFrame.Scale, scaleFactor);

            return Maths::Matrix4::Identity.Scaled(Maths::Vector4(finalScale, 1.0f));
        }

        void Animator::Reset()
        {
            m_currentAnimationTime = 0;
        }
    }
}